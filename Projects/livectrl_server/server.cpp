
#include "libwebsockets.h"
#include "util.h"
#include "ipc.h"

namespace Server
{
    /** per session structure */
    struct pss_live {
        struct lws           *wsi;            // http/ws ����
        bool                  send_header;    // Ӧ���Ƿ�д��header
        string               *response_body;
    };

    int callback_ctrl(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
    {
        pss_live *pss = (pss_live*)user;

        switch (reason) {
        case LWS_CALLBACK_PROTOCOL_INIT:
            Log::debug("live protocol init");
            break;
        case LWS_CALLBACK_HTTP:     //�ͻ���ͨ��http����
            {
                uint8_t buf[LWS_PRE + 2048];    //����httpͷ����
                uint8_t *start = &buf[LWS_PRE]; //htppͷ���λ��
                uint8_t *end = &buf[sizeof(buf) - LWS_PRE - 1]; //��β��λ��
                uint8_t *p = start;

                pss->wsi = wsi;
                pss->response_body = new string();

                char path[MAX_PATH]={0};
                lws_hdr_copy(wsi, path, MAX_PATH, WSI_TOKEN_GET_URI);
                Log::debug("new http-live request: %s", path);

                if(!strcmp(path, "/device/clients")) {
                    *pss->response_body = IPC::GetClientsJson();
                } else if(!strcmp(path, "/device/devlist")) {
                    *pss->response_body = IPC::GetDevsJson();
                } else if(!strcmp(path, "/device/refresh")) {
                    IPC::DevsFresh();
                    *pss->response_body = "ok";
                } else if(!strncmp(path, "/device/control", 8)) {
                    // �豸ID
                    char szDev[30]={0};
                    sscanf(path, "/device/control/%s", szDev);

                    // ����
                    int ud=0, lr=0, io=0;
                    char buf[20];
                    int n = 0;
                    while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n) > 0) {
                        char arg[10]={0};
                        int argv = 0;
                        sscanf(buf, "%[^=]=%d", arg, &argv);
                        if(!strcmp(arg, "ud")) {
                            ud = argv;
                        } else if(!strcmp(arg, "lr")) {
                            lr = argv;
                        } else if(!strcmp(arg, "io")) {
                            io = argv;
                        }
                        n++;
                    }
                    IPC::DevControl(szDev, io, ud, lr);
                    *pss->response_body = "ok";
                }

                lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html", pss->response_body->size(), &p, end);
                lws_add_http_header_by_name(wsi, (const uint8_t *)"Access-Control-Allow-Origin", (const uint8_t *)"*", 1, &p, end);
                if (lws_finalize_write_http_header(wsi, start, &p, end))
                    return 1;

                lws_callback_on_writable(wsi);
                return 0;
            }
        case LWS_CALLBACK_HTTP_WRITEABLE: //Http��������
            {
                if (!pss)
                    break;

                lws_write(wsi, (uint8_t *)pss->response_body->c_str(), pss->response_body->size(), LWS_WRITE_HTTP_FINAL);
                if (lws_http_transaction_completed(wsi))
                    return -1;

                return 0;
            }
        case LWS_CALLBACK_CLOSED_HTTP:  //Http���ӶϿ�
            {
                if (!pss)
                    break;
                delete pss->response_body;
            }
        default:
            break;
        }

        return lws_callback_http_dummy(wsi, reason, user, in, len);
    }

    static struct lws_context_creation_info info;  //libwebsockets������Ϣ
    static struct lws_context *context;            //libwebsockets���

    static struct lws_protocols protocols[] = {
        { "ctrl",   callback_ctrl,   sizeof(pss_live), 0 },
        { NULL, NULL, 0, 0 } 
    };

    // ״̬
    static bool _running = false;
    static bool _stop = false;

    //����libwebsockets�����־
    void userLog(int level, const char* line) {
        if(level & LLL_ERR)
            Log::error(line);
        else if(level & LLL_WARN)
            Log::warning(line);
        else if(level & LLL_NOTICE)
            Log::debug(line);
        else
            Log::debug(line);
    }

    int Init(void* uv, int port) {
        //������־
        int level = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
        lws_set_log_level(level, userLog);

        //����libwebsockets����
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
		info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
		info.foreign_loops = (void**)&uv;
		info.timeout_secs = 30;
		info.timeout_secs_ah_idle = 5;
        context = lws_create_context(&info);
        Log::debug("live sever start success");

		info.port = port;
        info.protocols = protocols;
		info.mounts = NULL;
        info.vhost_name = "live server";
		if (!lws_create_vhost(context, &info)) {
            Log::error("Failed to create http vhost\n");
            return -1;
        }

        return 0;
    }

    int Cleanup() {
        _running = false;
        while (!_stop) {
            Sleep(10);
        }
        return 0;
    }
};