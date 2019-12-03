#pragma once
#include "ring_buff.h"
#include <string>

namespace Server
{
    struct pss_live;
    enum MediaType;
    typedef struct _AV_BUFF_ AV_BUFF;

    class CLiveWorker
    {
    public:
        CLiveWorker();
        ~CLiveWorker();

        bool Play();

        /** ����˻�ȡ��Ƶ���� */
        int GetVideo(char **buff);

        void play_answer(int ret, std::string error_info);

        void push_ps_data(char* pBuff, int nLen);
        int get_ps_data(char* pBuff, int &nLen);

        void push_flv_frame(char* pBuff, int nLen);

        /**
         * �ײ�֪ͨ���Źر�(����rtp��ʱ���Է��رյ�)
         */
        void stop();

		void close();
    private:
        void cull_lagging_clients();

    public:
        pss_live     *m_pPss;           //< ���ӻỰ
        std::string           m_strCode;        //< ����ý����
        std::string           m_strType;        // Ŀ��ý������ flv mp4 h264
        std::string           m_strHw;          // Ŀ��ý��ֱ��� �ձ�ʾ����
        //HandleType            m_eHandleType;    //< ��������һ������
        std::string           m_strMIME;        //< mime type
        //MediaType             m_eMediaType;
        //int                   m_nChannel;       //< ͨ�� 0:ԭʼ����  1:С����
        bool                  m_bWebSocket;     //< false:http����true:websocket

        std::string           m_strPath;        //< ���Ŷ������ַ
        std::string           m_strClientName;  //< ���Ŷ˵�����
        std::string           m_strClientIP;    //< ���Ŷ˵�ip
        std::string           m_strError;       //< sip���������صĲ�������ʧ��ԭ��

    private:
        //void                 *m_pFormat;     //< ��Ƶ��ʽ���
        ring_buff_t          *m_pPSRing;       //< PS���ݶ���
        ring_buff_t          *m_pRing;         //< Ŀ���������ݶ���
        std::string           m_SocketBuff;    //< socket���͵����ݻ���
		bool                  m_bConnect;      //<
        bool                  m_bStop;

        //int                   m_nType;          //< 0:liveֱ����1:record��ʷ��Ƶ
    };

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(std::string strCode, std::string strType, std::string strHw, bool isWs, pss_live *pss);

    void InitFFmpeg();

};