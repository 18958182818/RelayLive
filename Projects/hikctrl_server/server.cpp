#include "util.h"
#include "libwebsockets.h"
#include "web.h"

namespace Server
{
	uv_loop_t *g_uv_loop = NULL;

    static struct lws_context_creation_info info;  //libwebsockets������Ϣ
    static struct lws_context *context;            //libwebsockets���

    // http��ַƥ��
    //static struct lws_http_mount mount_other;  //����
    static struct lws_http_mount mount_device; //�鿴�豸��Ϣ������
    //static struct lws_http_mount mount_web;    //վ�㾲̬�ļ�

    // ����������
    //static std::string mount_web_origin;  //վ�㱾��λ��
    //static std::string mount_web_def;     //Ĭ���ļ�
    //static int http_port = 80;            //HTTP����˿�

    static struct lws_protocols protocols[] = {
        { "http",   callback_other_http,  sizeof(pss_other),   0 },
        { "device", callback_device_http, sizeof(pss_device),  0 },
        { NULL, NULL, 0, 0 } 
    };

    //����libwebsockets�����־
    void userLog(int level, const char* line)
    {
        if(level & LLL_ERR)
            Log::error(line);
        else if(level & LLL_WARN)
            Log::warning(line);
        else if(level & LLL_NOTICE)
            Log::debug(line);
        else
            Log::debug(line);
    }

    int Init(void* uv, int port)
    {
		g_uv_loop = (uv_loop_t *)uv;

        //������־
        int level = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
        lws_set_log_level(level, userLog);

        // �豸��Ϣ�鿴����������
        memset(&mount_device, 0, sizeof(mount_device));
        mount_device.mountpoint = "/device";
        mount_device.mountpoint_len = 7;
        mount_device.origin_protocol = LWSMPRO_CALLBACK;
        mount_device.protocol = "device";
        //mount_device.mount_next = &mount_other;

        //����libwebsockets����
        memset(&info, 0, sizeof info);
        info.pcontext = &context;
        info.options = LWS_SERVER_OPTION_LIBUV | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
        info.foreign_loops = (void**)&g_uv_loop;
		info.timeout_secs = 0x1fffffff;
		info.timeout_secs_ah_idle = 0x1fffffff;
        context = lws_create_context(&info);

        //����http������
        info.port = port;
        info.protocols = protocols;
        info.mounts = &mount_device;
        info.vhost_name = "hik ctrl server";
        if (!lws_create_vhost(context, &info)) {
            Log::error("Failed to create http vhost\n");
            return -1;
        }

        return 0;
    }

    int Cleanup()
    {
        // ����libwebsockets
        lws_context_destroy(context);
        return 0;
    }
};