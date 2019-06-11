#ifndef AVFORMAT_RTSP
#define AVFORMAT_RTSP

#include "cstl.h"

typedef enum _rtsp_method_ {
    RTSP_ERROR = 0,
    RTSP_OPTIONS,
    RTSP_DESCRIBE,
    RTSP_SETUP,
    RTSP_PLAY,
    RTSP_PAUSE,
    RTSP_TEARDOWN,
}rtsp_method;

typedef struct _rtsp_ruquest_
{
    rtsp_method     method;
    char            *uri;
    uint32_t        CSeq;
    uint32_t        rtp_port;
    uint32_t        rtcp_port;
    hash_map_t      *headers; //map<string,string>
}rtsp_ruquest_t;

typedef struct _rtsp_ rtsp;
typedef void(*rtsp_callback)(void *user, rtsp_ruquest_t *req);

/**
 * ����һ��rtsp�������
 */
extern rtsp* create_rtsp(void *user, rtsp_callback cb);

/**
 * ��������һ��rtsp�������
 */
extern void destory_rtsp(rtsp *h);

/**
 * ��������socket���յ���������
 */
extern void rtsp_handle_request(rtsp *h, char *data, int len);

/**
 * ����ͻ���socket���յ���Ӧ������
 */
extern void rtsp_handle_answer(rtsp *h, char *data, int len);

#endif