#pragma once
#include "libLive.h"
enum flv_tag_type;
enum NalType;
enum MP4_FRAG_TYPE;

namespace HttpWsServer
{
    struct pss_http_ws_live;
    enum MediaType;

#define BASE_BUFF \
    char *pBuff;\
    int   nLen;

    struct BASIC_BUFF {
        BASE_BUFF
    };

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

    /**
     * MP4 box����
     */
    struct MP4_FRAG_BUFF {
        BASE_BUFF
        MP4_FRAG_TYPE eType;
    };

    class CLiveWorker : public IlibLiveCb
    {
    public:
        CLiveWorker(string strCode, int rtpPort);
        ~CLiveWorker();

        /** �ͻ������� */
        bool AddConnect(pss_http_ws_live* pss);
        bool DelConnect(pss_http_ws_live* pss);

        /** ��Դ��������Ƶ���ݣ����߳����� */
        void push_flv_frame(int tag_type, char* pBuff, int nLen);
        void push_h264_stream(NalType eType, char* pBuff, int nLen);
        void push_ts_stream(char* pBuff, int nLen);
        void push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize);

        /** ����˻�ȡ��Ƶ���� */
        FLV_TAG_BUFF GetFlvHeader();
        FLV_TAG_BUFF GetFlvVideo(uint32_t *tail);
        //------------------------------------------
        H264_NALU_BUFF GetH264Video(uint32_t *tail);
        //------------------------------------------
        MP4_FRAG_BUFF GetMp4Header();
        MP4_FRAG_BUFF GetMp4Video(uint32_t *tail);
        //------------------------------------------
        void NextWork(pss_http_ws_live* pss);

        /** ��ȡ�ͻ�����Ϣ */
        string GetClientInfo();
    private:
        void cull_lagging_clients(MediaType type);

        void stop();

    private:
        string                m_strCode;     // ����ý����

        //flv
        FLV_TAG_BUFF          m_stFlvScript;     // ScriptTag��ֻ��һ��
        FLV_TAG_BUFF          m_stFlvDecCof;     // ��һ��VideoTag������sps��pps���ɵ�AVCDecoderConfigurationRecord
        struct lws_ring       *m_pFlvRing;
        pss_http_ws_live      *m_pFlvPssList;

        //h264
        struct lws_ring       *m_pH264Ring;
        pss_http_ws_live      *m_pH264PssList;

        //fMP4
        MP4_FRAG_BUFF         m_stMP4Head;
        struct lws_ring       *m_pMP4Ring;
        pss_http_ws_live      *m_pMP4PssList;

        int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
        IlibLive*             m_pLive;          //< ֱ�����ݽ��պͽ��װ��
        int                   m_nPort;          //< rtp���ն˿�
    };

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(string strCode);
    CLiveWorker* GetLiveWorker(string strCode);
    bool DelLiveWorker(string strCode);

    /** �㲥 */
};