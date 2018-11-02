#include "stdafx.h"
#include "libwebsockets.h"
#include "HttpLiveServer.h"
#include "LiveWorker.h"
// ����ģ������
#include "SipInstance.h"
#include "H264.h"
#include "Flv.h"

namespace HttpWsServer
{
    static string StartPlayLive(pss_http_ws_live *pss, string devCode)
    {
        if(pss == nullptr)
        {
            return g_strError_play_faild;
        }

        CLiveWorker* pWorker = GetLiveWorker(devCode);
        if (!pWorker) {
            pWorker = CreatLiveWorker(devCode);
        }
        if(!pWorker) {
            Log::error("CreatFlvBuffer failed%s", devCode.c_str());
            return g_strError_play_faild;
        }
        pWorker->AddConnect(pss);

        pss->m_pWorker = pWorker;
        return "";
    }

    static void SendLiveFlv(pss_http_ws_live *pss) {
        int len = 0;
        int wlen = 0;
        CLiveWorker* pWorker = (CLiveWorker*)pss->m_pWorker;

        FLV_TAG_BUFF flvVideo = pWorker->GetFlvVideo(&pss->tail);
        if (!pss->m_bSendHead) {
            Log::debug("first flv data with header: tail:%d",pss->tail);
            FLV_TAG_BUFF flvheader = pWorker->GetFlvHeader();
            if(flvheader.pBuff != nullptr && flvVideo.pBuff != nullptr) {
                if(flvVideo.eType == callback_key_video_tag) {
                    Log::debug("send key tag");
                    len = flvheader.nLen + flvVideo.nLen;
                    uint8_t* buff = (uint8_t *)malloc(len + LWS_PRE);
                    int nPos = LWS_PRE;
                    memcpy(buff + nPos, flvheader.pBuff, flvheader.nLen);
                    nPos += flvheader.nLen;
                    memcpy(buff + nPos, flvVideo.pBuff, flvVideo.nLen);
                    wlen = lws_write(pss->wsi, (uint8_t *)buff + LWS_PRE, len, LWS_WRITE_BINARY);
                    pss->m_bSendHead = true;
                    free(buff);
                    pWorker->NextWork(pss);
                }
            }
        } else {
            Log::debug(" flv data tail:%d", pss->tail);
            len = flvVideo.nLen;
            uint8_t *buff = (uint8_t *)malloc(len+LWS_PRE);
            memcpy(buff+LWS_PRE, flvVideo.pBuff, len);
            wlen = lws_write(pss->wsi, (uint8_t *)buff+LWS_PRE, len, LWS_WRITE_BINARY);
            free(buff);
            pWorker->NextWork(pss);
        }
    }

    static void SendLiveH264(pss_http_ws_live *pss) {
        H264_NALU_BUFF tag = pss->m_pWorker->GetH264Video(&pss->tail);
        uint8_t *buff = (uint8_t *)malloc(tag.nLen+LWS_PRE);
        memcpy(buff+LWS_PRE, tag.pBuff, tag.nLen);
        int wlen = lws_write(pss->wsi, (uint8_t *)buff+LWS_PRE, tag.nLen, LWS_WRITE_BINARY);
        free(buff);
        pss->m_pWorker->NextWork(pss);
    }

    int callback_live_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        pss_http_ws_live *pss = (pss_http_ws_live*)user;

        switch (reason) {
        case LWS_CALLBACK_HTTP: 
            {
                uint8_t buf[LWS_PRE + 2048],
                    *start = &buf[LWS_PRE],
                    *p = start,
                    *end = &buf[sizeof(buf) - LWS_PRE - 1];

                lws_snprintf(pss->path, sizeof(pss->path), "%s", (const char *)in);
                Log::debug("new request: %s", pss->path);

                pss->wsi = wsi;
                pss->pss_next = nullptr;
                pss->isWs = false;
                pss->m_bSendHead = false;
                pss->m_pWorker = nullptr;
                lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi),
                    pss->clientName, sizeof(pss->clientName),
                    pss->clientIP, sizeof(pss->clientIP));


                string strErrInfo;
                string strPath = pss->path;
                do{
                    size_t pos = strPath.rfind(".");
                    if(pos == string::npos) {
                        strErrInfo = "error request path";
                        break;
                    }
                    string uri_type = strPath.substr(pos+1, strPath.size()-pos-1);
                    string devcode = strPath.substr(1, pos -1);

                    if (!_stricmp(uri_type.c_str(), "flv")) {
                        pss->media_type = media_flv;
                        strErrInfo = StartPlayLive(pss, devcode);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else if(!_stricmp(uri_type.c_str(), "hls")) {
                        pss->media_type = media_hls;
                        strErrInfo = "error request path";
                        break;
                    } else if(!_stricmp(uri_type.c_str(), "h264")) {
                        pss->media_type = media_h264;
                        strErrInfo = StartPlayLive(pss, devcode);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else if(!_stricmp(uri_type.c_str(), "mp4")) {
                        pss->media_type = media_mp4;
                        strErrInfo = "error request path";
                        break;
                    } else {
                        strErrInfo = "error request path";
                        break;
                    }
                    lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Content-Type", (const uint8_t *)"video/x-flv", 11, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Cache-Control", (const uint8_t *)"no-cache", 8, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Expires", (const uint8_t *)"-1", 2, &p, end);
                    lws_add_http_header_by_name(wsi, (const uint8_t *)"Pragma", (const uint8_t *)"no-cache", 8, &p, end);
                    if (lws_finalize_write_http_header(wsi, start, &p, end))
                        return 1;

                    //lws_callback_on_writable(wsi);
                    return 0;
                }while(0);

                lws_snprintf(pss->strErrInfo, sizeof(pss->strErrInfo), "%s", strErrInfo.c_str());

                if (lws_add_http_common_headers(wsi, HTTP_STATUS_FORBIDDEN, "text/html",
                    LWS_ILLEGAL_HTTP_CONTENT_LEN, &p, end))
                    return 1;
                if (lws_finalize_write_http_header(wsi, start, &p, end))
                    return 1;

                lws_callback_on_writable(wsi);
                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE: 
            {
                if (!pss || pss->culled)
                    break;

                if (strlen(pss->strErrInfo) == 0) {

                    if(pss->media_type == media_flv){
                        SendLiveFlv(pss);
                    } 
                    else if(pss->media_type == media_h264) {
                        SendLiveH264(pss);
                    }
                    //lws_callback_on_writable(wsi);
                    return 0;
                } else {
                    int len = strlen(pss->strErrInfo);
                    int wlen = lws_write(wsi, (uint8_t *)pss->strErrInfo, len, LWS_WRITE_HTTP_FINAL);
                    if (wlen != len)
                        return 1;
                    if (lws_http_transaction_completed(wsi))
                        return -1;
                    return 0;
                }

                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:
            {
                if (!pss)
                    break;
                pss->m_pWorker->DelConnect(pss);
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
                /* add ourselves to the list of live pss held in the vhd */
                lws_hdr_copy(wsi, pss->path, sizeof(pss->path), WSI_TOKEN_GET_URI);
                Log::debug("live ws protocol establised: %s", pss->path);

                pss->wsi = wsi;
                pss->pss_next = nullptr;
                pss->isWs = true;
                pss->m_bSendHead = false;
                pss->m_pWorker = nullptr;
                lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi),
                    pss->clientName, sizeof(pss->clientName),
                    pss->clientIP, sizeof(pss->clientIP));

                string strErrInfo;
                string strPath = pss->path;
                do{
                    size_t pos = strPath.rfind(".");
                    if(pos == string::npos) {
                        strErrInfo = "error request path";
                        break;
                    }
                    string uri_type = strPath.substr(pos+1, strPath.size()-pos-1);
                    string devcode = strPath.substr(1, pos -1);

                    if (!_stricmp(uri_type.c_str(), "flv")) {
                        pss->media_type = media_flv;
                        strErrInfo = StartPlayLive(pss, devcode);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else if(!_stricmp(uri_type.c_str(), "hls")) {
                        pss->media_type = media_hls;
                        strErrInfo = "error request path";
                        break;
                    } else if(!_stricmp(uri_type.c_str(), "h264")) {
                        pss->media_type = media_h264;
                        strErrInfo = StartPlayLive(pss, devcode);
                        if(!strErrInfo.empty()){
                            break;
                        }
                    } else if(!_stricmp(uri_type.c_str(), "mp4")) {
                        pss->media_type = media_mp4;
                        strErrInfo = "error request path";
                        break;
                    } else {
                        strErrInfo = "error request path";
                        break;
                    }

                    return 0;
                }while(0);

                //����ʧ�ܣ��Ͽ�����
                lws_snprintf(pss->strErrInfo, sizeof(pss->strErrInfo), "%s", (const char *)strErrInfo.c_str());
                int len = strlen(pss->strErrInfo);
                lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL,(uint8_t *)pss->strErrInfo, len);
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
                if (!pss || pss->culled)
                    break;

                //Log::debug("live ws protocol writeable %s", pss->path);
                int len = 0;
                int wlen = 0;
                if(pss->media_type == media_flv){
                    SendLiveFlv(pss);
                } 
                else if(pss->media_type == media_h264) {
                    SendLiveH264(pss);
                }

                return 0;
            }
        case LWS_CALLBACK_CLOSED:
            {
                if (!pss)
                    break;
                Log::debug("live ws protocol cloes %s", pss->path);
                pss->m_pWorker->DelConnect(pss);
            }
        default:
            break;
        }

        return 0;
    }
};