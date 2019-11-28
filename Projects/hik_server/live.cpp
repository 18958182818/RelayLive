
#include "libwebsockets.h"
#include "live.h"
#include "worker.h"
#include "util.h"

namespace Server
{
    enum live_error {
        live_error_ok = 0,
        live_error_uri,
        live_error_media_type
    };

    static const char* live_error_str[] = {
        "ok",
        "error request uri",
        "unkown media type"
    };

    static bool ParseRequest(pss_http_ws_live *pss, bool websocket) {
        /* http(ws)://IP:port/live?code=123456789&type=flv&hw=640*480
           http(ws)://IP:port/live?url=AAAAAAA&type=flv&&hw=640*480
           type: flv��mp4��h264��hls
           ����pathֻ�� /live/type/stream/code
        */
        char path[MAX_PATH]={0};
        lws_hdr_copy(pss->wsi, path, MAX_PATH, WSI_TOKEN_GET_URI);
        if(websocket)
            Log::debug("new ws-live protocol establised: %s", path);
        else
            Log::debug("new http-live request: %s", path);

        string strPath = path;
        size_t parampos = strPath.find('?');
        if(parampos == string::npos) {
            Log::error("%s error uri", path);
            pss->error_code = live_error_uri;
            return false;
        }
        strPath = strPath.substr(parampos+1, strPath.size()-parampos-1);
        string strCode, strUrl, strHw, strType="flv";
        vector<string> vecpars = StringHandle::StringSplit(strPath, '&');
        for(auto par:vecpars) {
            vector<string> tmp = StringHandle::StringSplit(par, '=');
            if(tmp.size() != 2)
                continue;
            if(tmp[0] == "code")
                strCode = tmp[1];
            else if(tmp[0] == "url")
                strUrl = tmp[1];
            else if(tmp[0] == "type")
                strType = tmp[1];
            else if(tmp[0] == "hw")
                strHw = tmp[1];
        }

        pss->pWorker = CreatLiveWorker(strCode, strType, strHw, false, pss);
        
        //ĳЩ�����£���ȡsocket�Զ�ip��Ȼ��ʱ�ܶ�
        char clientName[50]={0};    //���Ŷ˵�����
        char clientIP[50]={0};      //���Ŷ˵�ip
        lws_get_peer_addresses(pss->wsi, lws_get_socket_fd(pss->wsi),
            clientName, sizeof(clientName),
            clientIP, sizeof(clientIP));
        pss->pWorker->m_strClientIP = clientIP;
        return true;
    }

    static bool WriteHeader(pss_http_ws_live *pss) {
        uint8_t buf[LWS_PRE + 2048];    //����httpͷ����
        uint8_t *start = &buf[LWS_PRE]; //htppͷ���λ��
        uint8_t *end = &buf[sizeof(buf) - LWS_PRE - 1]; //��β��λ��
        uint8_t *p = start;
        struct lws *wsi = pss->wsi;
        pss->send_header = true;
        if(!pss->pWorker->m_bWebSocket && !pss->error_code){
            // http �ɹ�
            lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end);
            lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
            lws_add_http_header_by_name(wsi, (const uint8_t *)"Content-Type", (const uint8_t *)pss->pWorker->m_strMIME.c_str(), pss->pWorker->m_strMIME.size(), &p, end);
            lws_add_http_header_by_name(wsi, (const uint8_t *)"Cache-Control", (const uint8_t *)"no-cache", 8, &p, end);
            lws_add_http_header_by_name(wsi, (const uint8_t *)"Expires", (const uint8_t *)"-1", 2, &p, end);
            lws_add_http_header_by_name(wsi, (const uint8_t *)"Pragma", (const uint8_t *)"no-cache", 8, &p, end);
            if(lws_finalize_write_http_header(wsi, start, &p, end))
                return false;
        } else if(!pss->pWorker->m_bWebSocket) {
            // http ʧ��
            if (lws_add_http_common_headers(wsi, HTTP_STATUS_FORBIDDEN, "text/html",
                LWS_ILLEGAL_HTTP_CONTENT_LEN, &p, end))
                return false;
            if (lws_finalize_write_http_header(wsi, start, &p, end))
                return false;
            lws_callback_on_writable(wsi);
        } else if(!pss->error_code) {
            // websocket �ɹ�
        } else {
            // websocket ʧ��
            //����ʧ�ܣ��Ͽ�����
            const char *errors = live_error_str[pss->error_code];
            int len = strlen(errors);
            lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL,(uint8_t *)errors, len);
        }
        return true;
    }
 

    static bool SendBody(pss_http_ws_live *pss){
        char *buff;
        CLiveWorker *pWorker = (CLiveWorker *)pss->pWorker;
        int nLen = pWorker->GetVideo(&buff);
        if(nLen <= LWS_PRE)
            return true;

        int wlen = lws_write(pss->wsi, (uint8_t *)buff + LWS_PRE, nLen-LWS_PRE, LWS_WRITE_BINARY);
        lws_callback_on_writable(pss->wsi);

        return true;
    }

    int callback_live_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        pss_http_ws_live *pss = (pss_http_ws_live*)user;

        switch (reason) {
        case LWS_CALLBACK_HTTP: 
            {
                pss->wsi = wsi;
                
                if(!ParseRequest(pss, false)){
                    WriteHeader(pss);
                }

                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE: 
            {
                if (!pss || !pss->pWorker)
                    break;

                if(!pss->send_header) {
                    WriteHeader(pss);
                    return 0;
                }

                if (pss->error_code == 0) {
                    if(!SendBody(pss))
                        return -1;
                } else {
                    const char *errors = live_error_str[pss->error_code];
                    int len = strlen(errors);
                    lws_write(wsi, (uint8_t *)errors, len, LWS_WRITE_HTTP_FINAL);
                    if (lws_http_transaction_completed(wsi))
                        return -1;
                }

                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                if (!pss || !pss->pWorker)
                    break;
                pss->pWorker->close();
            }
        default:
            break;
        }

        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }

    int callback_live_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        pss_http_ws_live *pss = (pss_http_ws_live*)user;

        switch (reason) 
        {
        case LWS_CALLBACK_PROTOCOL_INIT:
            Log::debug("live ws protocol init");
            break;
        case LWS_CALLBACK_ESTABLISHED:
            {
                pss->wsi = wsi;

                if(!ParseRequest(pss, true)) {
                    WriteHeader(pss);
                }
                return 0;
            }
            break;
        case LWS_CALLBACK_RECEIVE: 
            {
                if (!pss)
                    break;
                string strRecv((char*)in,len);
                Log::debug("live ws protocol recv len:%d", len);
                Log::debug(strRecv.c_str());
            }
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE: 
            {
                if (!pss || !pss->pWorker)
                    break;

                if(!pss->send_header) {
                    WriteHeader(pss);
                    return 0;
                }

                //Log::debug("live ws protocol writeable %s", pss->path);
                if(!SendBody(pss))
                    return -1;

                return 0;
            }
        case LWS_CALLBACK_CLOSED:
            {
                if (!pss || !pss->pWorker)
                    break;
				Log::debug("live ws protocol cloes %s", pss->pWorker->m_strPath.c_str());
				pss->pWorker->close();
            }
        default:
            break;
        }

        return 0;
    }
};