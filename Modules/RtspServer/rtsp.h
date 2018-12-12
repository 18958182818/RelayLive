#pragma once
#include "uv.h"

enum devType {
    DEV_HIK = 0,
    DEV_DAHUA
};

struct RTSP_REQUEST
{
    string     ip;
    uint32_t   port;
    string     user_name;
    string     password;
    uint32_t   channel;     //ͨ���ţ�1��2...��
    uint32_t   stream;      //���� 0:������ 1:������
    uint32_t   rtp_port;
    devType    dev_type;    //�豸����
};

extern int set_uv(uv_loop_t* uv);

extern void stop_uv();

/**
 * ��������
 */
typedef void (*play_cb)(int status);
extern int rtsp_play(RTSP_REQUEST config);

/**
 * �ر�rtsp
 */
extern int rtsp_stop(string ip);