#pragma once
#include "ring_buff.h"
#include "util.h"
#include <string>
#include <list>

namespace Server
{
    struct live_session;
    enum MediaType;

    class CLiveWorker
    {
    public:
        CLiveWorker();
        ~CLiveWorker();

        bool Play();

        void push_ps_data(char* pBuff, int nLen);
        int get_ps_data(char* pBuff, int &nLen);

        void push_flv_frame(char* pBuff, int nLen);
        int get_flv_frame(char **buff);   /** ����˻�ȡ��Ƶ���� */
		void next_flv_frame();

		void close();
    private:
        void cull_lagging_clients();
		bool is_key(char* pBuff, int nLen);

    public:
        live_session         *m_pPss;           //< ���ӻỰ
        std::string           m_strCode;        //< ����ý����
        std::string           m_strType;        // Ŀ��ý������ flv mp4 h264
        //std::string           m_strHw;          // Ŀ��ý��ֱ��� �ձ�ʾ����
        int                   m_nWidth;
        int                   m_nHeight;
        std::string           m_strMIME;        //< mime type
        bool                  m_bWebSocket;     //< false:http����true:websocket

        std::string           m_strPath;        //< ���Ŷ������ַ
        std::string           m_strClientName;  //< ���Ŷ˵�����
        std::string           m_strClientIP;    //< ���Ŷ˵�ip
        std::string           m_strError;       //< sip���������صĲ�������ʧ��ԭ��

    private:
        ring_buff_t          *m_pPSRing;       //< PS���ݶ���
        ring_buff_t          *m_pFlvRing;      //< Ŀ���������ݶ���
        std::string           m_SocketBuff;    //< socket���͵����ݻ���
		bool                  m_bConnect;      //< �ͻ�������״̬
		bool                  m_bParseKey;     //< 
		char                 *m_pTmpBuff;
		int                   m_nTmpBuffSize;
		int                   m_nTmpBuffTotalSize;
		int                   m_nTmpBuffReaded;
    };

    /** ֱ�� */
    CLiveWorker* CreatLiveWorker(std::string strCode, std::string strType, std::string strHw, bool isWs, live_session *pss, string clientIP);

    void InitFFmpeg();

};