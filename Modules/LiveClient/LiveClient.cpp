#include "stdafx.h"
#include "LiveClient.h"
#include "LiveIpc.h"
#include "LiveWorker.h"

namespace LiveClient
{
    extern string g_strRtpIP;            //< RTP����IP
    extern int    g_nRtpBeginPort;       //< RTP��������ʼ�˿ڣ�������ż��
    extern int    g_nRtpPortNum;         //< RTPʹ�õĸ�������strRTPPort��ʼÿ�μ�2����strRTPNum��
    extern int    g_nRtpCatchPacketNum;  //< rtp����İ�������
    extern int    g_nRtpStreamType;      //< rtp�������ͣ�����libLive��ps h264

    extern vector<int>     m_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���
    extern CriticalSection m_csRTP;          //< RTP�˿���

    extern LIVECLIENT_CB ipc_cb;

    void Init(){
        /** ���̼�ͨ�� */
        LiveIpc::Init();

        /** �������� */
        g_strRtpIP           = Settings::getValue("RtpClient","IP");                    //< RTP����IP
        g_nRtpBeginPort      = Settings::getValue("RtpClient","BeginPort",10000);       //< RTP��������ʼ�˿ڣ�������ż��
        g_nRtpPortNum        = Settings::getValue("RtpClient","PortNum",1000);          //< RTPʹ�õĸ�������strRTPPort��ʼÿ�μ�2����strRTPNum��
        g_nRtpCatchPacketNum = Settings::getValue("RtpClient", "CatchPacketNum", 100);  //< rtp����İ�������
        g_nRtpStreamType     = Settings::getValue("RtpClient", "Filter", 0);            //< rtp���ͣ���ps����h264

        Log::debug("RtpConfig IP:%s, BeginPort:%d,PortNum:%d,CatchPacketNum:%d"
            , g_strRtpIP.c_str(), g_nRtpBeginPort, g_nRtpPortNum, g_nRtpCatchPacketNum);
        m_vecRtpPort.clear();
        for (int i=0; i<g_nRtpPortNum; ++i) {
            m_vecRtpPort.push_back(g_nRtpBeginPort+i*2);
        }

        if(g_nRtpStreamType == 0)
            g_stream_type = STREAM_PS;
        else if(g_nRtpStreamType == 1)
            g_stream_type = STREAM_H264;
    }

    string GetClientsInfo() 
    {
        //MutexLock lock(&m_cs);
        //auto it = m_workerMap.begin();
        //auto end = m_workerMap.end();
        string strResJson = "{\"root\":[";
        //for (;it != end; ++it)
        //{
        //    strResJson += it->second->GetClientInfo();
        //}
        //strResJson = StringHandle::StringTrimRight(strResJson,',');
        //strResJson += "]}";
        return strResJson;
    }

    void GetDevList(){
        LiveIpc::GetDevList();
    }

    void QueryDirtionary(){
        LiveIpc::QueryDirtionary();
    }

    void SetCallBack(LIVECLIENT_CB cb){
        ipc_cb = cb;
    }

    ILiveWorker* GetWorker(string strCode){
        CLiveWorker* worker = GetLiveWorker(strCode);
        if(nullptr == worker)
            worker = CreatLiveWorker(strCode);
        return (ILiveWorker*)worker;
    }
}