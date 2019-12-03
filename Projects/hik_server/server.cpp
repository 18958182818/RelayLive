
#include "libwebsockets.h"
#include "live.h"
#include "worker.h"
#include "util.h"

namespace Server
{
    static struct lws_context_creation_info info;  //libwebsockets������Ϣ
    static struct lws_context *context;            //libwebsockets���

    static struct lws_protocols protocols[] = {
        { "live",   callback_live,   sizeof(pss_live), 0 },
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

    int Init(int port) {
        InitFFmpeg();
        //������־
        int level = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
        lws_set_log_level(level, userLog);

        //����libwebsockets����
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
		info.timeout_secs = 0x1fffffff;
		info.timeout_secs_ah_idle = 0x1fffffff;
        info.port = port;
        info.protocols = protocols;
        info.vhost_name = "hik server";
        context = lws_create_context(&info);
        Log::debug("hik_tmc_sdk sever start success");

        int n=0;
        _running = true;
        while (n >= 0 && _running){
            n=lws_service(context, 0x1fffffff);
        }

        // ����libwebsockets
        lws_context_destroy(context);
        _stop = true;
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