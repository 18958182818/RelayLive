#pragma once
#include "libLive.h"
enum flv_tag_type;
enum NalType;

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

    //enum flv_tag_type
    //{
    //    callback_flv_header = 0,
    //    callback_script_tag,
    //    callback_video_spspps_tag,
    //    callback_key_video_tag,
    //    callback_video_tag
    //};

#define BASE_BUFF \
    char *pBuff;\
    int   nLen;

    /**
     * FLV tag����
     */
    struct FLV_TAG_BUFF {
        BASE_BUFF
        flv_tag_type  eType; // �����Ƶtag�Ƿ���sps_pps,�ؼ�֡���ǹؼ�֡
    };

    /**
     * H264 nalu����
     */
    struct H264_NALU_BUFF {
        BASE_BUFF
        NalType eType; //h264֡����
    };

    class CLiveWorker : public IlibLiveCb
    {
    public:
        CLiveWorker(string strCode);
        ~CLiveWorker();

        /** �ͻ������� */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** ��Դ��������Ƶ���ݣ����߳����� */
        void push_flv_frame(int tag_type, char* pBuff, int nLen);
        void push_h264_stream(NalType eType, char* pBuff, int nLen);
        void push_ts_stream(char* pBuff, int nLen);

        /** ����˻�ȡ��Ƶ���� */
        FLV_TAG_BUFF GetFlvHeader();
        FLV_TAG_BUFF GetFlvVideo(uint32_t *tail);
        H264_NALU_BUFF GetH264Video(uint32_t *tail);
        void NextWork(pss_http_ws_live* pss);

        /** ��ȡ�ͻ�����Ϣ */
        string GetClientInfo();
    private:
        void cull_lagging_clients(MediaType type);

        void stop();

    private:
        string                m_strCode;     // ����ý����

        //flv
        FLV_TAG_BUFF          m_pScriptTag;     // ScriptTag��ֻ��һ��
        FLV_TAG_BUFF          m_pDecodeConfig;  // ��һ��VideoTag������sps��pps���ɵ�AVCDecoderConfigurationRecord
        struct lws_ring       *m_flvRing;
        pss_http_ws_live      *m_flvPssList;

        //h264
        struct lws_ring       *m_h264Ring;
        pss_http_ws_live      *m_h264PssList;

        int                   m_type;           //0:liveֱ����1:record��ʷ��Ƶ
        IlibLive*             m_pLive;          //< ֱ�����ݽ��պͽ��װ��
        int                   m_nPort;          //< rtp���ն˿�
    };

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(string strCode);
    CLiveWorker* GetLiveWorker(string strCode);
    bool DelLiveWorker(string strCode);

    /** �㲥 */
};