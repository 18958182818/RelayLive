#include "stdafx.h"
#include "uv.h"
#include "LiveWorker.h"
#include "LiveClient.h"
#include "liveReceiver.h"
#include "LiveChannel.h"
#include "LiveIpc.h"

namespace LiveClient
{
	extern uv_loop_t *g_uv_loop;

    extern string g_strRtpIP;            //< RTP����IP
    extern int    g_nRtpBeginPort;       //< RTP��������ʼ�˿ڣ�������ż��
    extern int    g_nRtpPortNum;         //< RTPʹ�õĸ�������strRTPPort��ʼÿ�μ�2����strRTPNum��
    extern int    g_nRtpCatchPacketNum;  //< rtp����İ�������
	extern int    g_nRtpStreamType;      //< rtp�������ͣ�����libLive��ps h264
    extern int    g_nNodelay;            //< ��Ƶ��ʽ����Ƿ��������� 1:ÿ��֡����������  0:ÿ���ؼ�֡�������ķǹؼ�֡�յ���һ����

    extern vector<int>     m_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���
    //extern CriticalSection m_csRTP;          //< RTP�˿���

    static map<string,CLiveWorker*>  m_workerMap;
    //static CriticalSection           m_cs;

    static int GetRtpPort()
    {
        //MutexLock lock(&m_csRTP);

        int nRet = -1;
        auto it = m_vecRtpPort.begin();
        if (it != m_vecRtpPort.end()) {
            nRet = *it;
            m_vecRtpPort.erase(it);
        }

        return nRet;
    }

    static void GiveBackRtpPort(int nPort)
    {
        //MutexLock lock(&m_csRTP);
        m_vecRtpPort.push_back(nPort);
    }


    /** ��ʱ���ٶ�ʱ����loop���Ƴ���� */
    static void stop_timer_close_cb(uv_handle_t* handle) {
        CLiveWorker* live = (CLiveWorker*)handle->data;
        if (live->m_bStop){
            live->Clear2Stop();
        } else {
            Log::debug("new client comed, and will not close live stream");
        }
    }

    /** �ͻ���ȫ���Ͽ�����ʱ�Ͽ�Դ�Ķ�ʱ�� */
	static void stop_timer_cb(uv_timer_t* handle) {
		CLiveWorker* live = (CLiveWorker*)handle->data;
		int ret = uv_timer_stop(handle);
		if(ret < 0) {
			Log::error("timer stop error:%s",uv_strerror(ret));
        }
        live->m_bStop = true;
        uv_close((uv_handle_t*)handle, stop_timer_close_cb);
	}

    /** CLiveWorker������ɾ��m_pLive�ȽϺ�ʱ��������event loop�����ʹ���̡߳� */
    static void live_worker_destory_thread(void* arg) {
        CLiveWorker* live = (CLiveWorker*)arg;
        SAFE_DELETE(live);
    }

    //////////////////////////////////////////////////////////////////////////

    CLiveWorker* CreatLiveWorker(string strCode)
    {
        Log::debug("CreatFlvBuffer begin");
        int rtpPort = GetRtpPort();
        if(rtpPort < 0) {
            Log::error("play failed %s, no rtp port",strCode.c_str());
            return nullptr;
        }

        string sdp;
        if(LiveIpc::RealPlay(strCode, g_strRtpIP,  rtpPort, sdp))
        {
            //uv_thread_t tid;
            //uv_thread_create(&tid, live_worker_destory_thread, pNew);
            Log::error("play failed %s",strCode.c_str());
            return nullptr;
        }

        Log::debug("RealPlay ok: %s",strCode.c_str());
        CLiveWorker* pNew = new CLiveWorker(strCode, rtpPort, sdp);

        //MutexLock lock(&m_cs);
        m_workerMap.insert(make_pair(strCode, pNew));

        return pNew;
    }

    CLiveWorker* GetLiveWorker(string strCode)
    {
        //Log::debug("GetWorker begin");

        //MutexLock lock(&m_cs);
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // �Ѿ�����
            return itFind->second;
        }

        //Log::error("GetWorker failed: %s",strCode.c_str());
        return nullptr;
    }

    bool DelLiveWorker(string strCode)
    {
        //MutexLock lock(&m_cs);
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // CLiveWorker������ɾ��m_pLive�ȽϺ�ʱ��������event loop�����ʹ���߳����ٶ���
            uv_thread_t tid;
            uv_thread_create(&tid, live_worker_destory_thread, itFind->second);

            m_workerMap.erase(itFind);
            return true;
        }

        Log::error("dosn't find worker object");
        return false;
    }

	string GetAllWorkerClientsInfo(){
        stringstream ss;
		ss << "{\"root\":[";
        //MutexLock lock(&m_cs);
        for (auto w : m_workerMap) {
            CLiveWorker *worker = w.second;
			vector<ClientInfo> tmp = worker->GetClientInfo();
            for(auto c:tmp){
                ss << "{\"DeviceID\":\"" << c.devCode 
                    << "\",\"Connect\":\"" << c.connect
                    << "\",\"Media\":\"" << c.media
                    << "\",\"ClientIP\":\"" << c.clientIP
                    << "\",\"Channel\":\"" << c.channel
                    << "\"},";
            }
		}
        string strResJson = StringHandle::StringTrimRight(ss.str(),',');
        strResJson += "]}";
        return strResJson;
	}

    //////////////////////////////////////////////////////////////////////////

    static void H264DecodeCb(AV_BUFF buff, void* user) {
        CLiveWorker* live = (CLiveWorker*)user;
        live->ReceiveYUV(buff);
    }

    CLiveWorker::CLiveWorker(string strCode, int rtpPort, string sdp)
        : m_strCode(strCode)
        , m_nPort(rtpPort)
        , m_strSDP(sdp)
        , m_nType(0)
        , m_pReceiver(nullptr)
        , m_pOrigin(nullptr)
#ifdef USE_FFMPEG
        , m_pDecoder(nullptr)
#endif
        , m_bStop(false)
        , m_bOver(false)
    {
        //��sdp��������ƵԴip�Ͷ˿�
        size_t t1,t2;
        t1 = sdp.find("c=IN IP4 ");
        t1 += 9;
        t2 = sdp.find("\r\n", t1);
        string rip = sdp.substr(t1, t2-t1);
        t1 = sdp.find("m=video ");
        t1 += 8;
        t2 = sdp.find(" ", t1);
        string rport = sdp.substr(t1, t2-t1);
        int nport = stoi(rport);

        //��������
        m_pReceiver = new CLiveReceiver(rtpPort, this);
        m_pReceiver->m_strRemoteIP = rip;
        m_pReceiver->m_nRemoteRTPPort = nport;
        m_pReceiver->m_nRemoteRTCPPort = nport+1;
        m_pReceiver->StartListen();
    }

    CLiveWorker::~CLiveWorker()
    {
        string ssid = StringHandle::toStr<int>(m_nPort);
        if(LiveIpc::StopPlay(ssid)) {
            Log::error("stop play failed");
        }
        SAFE_DELETE(m_pReceiver);
        SAFE_DELETE(m_pOrigin);
        MutexLock lock(&m_csChls);
        for(auto it:m_mapChlEx){
            SAFE_DELETE(it.second);
        }
        m_mapChlEx.clear();
        GiveBackRtpPort(m_nPort);
        Log::debug("CLiveWorker release");
    }

    bool CLiveWorker::AddHandle(ILiveHandle* h, HandleType t, int c)
    {
        if(c == 0) {
            // ԭʼͨ��
            if(!m_pOrigin)
                m_pOrigin = new CLiveChannel;
            CHECK_POINT(m_pOrigin);
            m_pOrigin->AddHandle(h, t);
        } else {
            // ��չͨ��
            MutexLock lock(&m_csChls);
            auto fit = m_mapChlEx.find(c);
            if(fit != m_mapChlEx.end()) {
                fit->second->AddHandle(h, t);
            } else {
                CLiveChannel *nc = new CLiveChannel(c, 640, 480);
                nc->AddHandle(h, t);
#ifdef USE_FFMPEG
                nc->SetDecoder(m_pDecoder);
#endif
                m_mapChlEx.insert(make_pair(c, nc));
            }
        }

        //����ոտ����˽�����ʱ������Ҫ����ر�
        if(uv_is_active((const uv_handle_t*)&m_uvTimerStop)) {
            uv_timer_stop(&m_uvTimerStop);
            uv_close((uv_handle_t*)&m_uvTimerStop, stop_timer_close_cb);
        }
        return true;
    }

    bool CLiveWorker::RemoveHandle(ILiveHandle* h)
    {
        // ԭʼͨ��
        bool bOriginEmpty = true;
        if(m_pOrigin){
            bOriginEmpty = m_pOrigin->RemoveHandle(h);
            if(bOriginEmpty) {
                SAFE_DELETE(m_pOrigin);
            }
        }

        //��չͨ��
        bool bExEmpty = true;
        MutexLock lock(&m_csChls);
        for(auto it = m_mapChlEx.begin(); it != m_mapChlEx.end();) {
            bool bEmpty = it->second->RemoveHandle(h);
            if(bEmpty) {
                delete it->second;
                it = m_mapChlEx.erase(it);
            } else {
                it++;
                bExEmpty = false;
            }
        }

        if(bExEmpty && bOriginEmpty) {
            if(m_bOver) {
                // ��ƵԴû�����ݣ���ʱ������Ҫ��ʱ
                Clear2Stop();
            } else {
                // ��ƵԴ��Ȼ���ӣ���ʱN�������ٶ����Ա��ʱ�������������ܿ��ٲ���
                uv_timer_init(g_uv_loop, &m_uvTimerStop);
                m_uvTimerStop.data = this;
                uv_timer_start(&m_uvTimerStop, stop_timer_cb, 5000, 0);
            }
        }
        return true;
    }

    string CLiveWorker::GetSDP(){
        return m_strSDP;
    }

	void CLiveWorker::Clear2Stop() {
        bool bOriginEmpty = !m_pOrigin || m_pOrigin->Empty();

        MutexLock lock(&m_csChls);
        bool bExEmpty = m_mapChlEx.empty();


        if(bOriginEmpty && bExEmpty) {
			Log::debug("need close live stream");
            //���ȴ�map�����߶���
            DelLiveWorker(m_strCode);
		}
	}

    void CLiveWorker::stop()
    {
        //��ƵԴû�����ݲ���ʱ
        Log::debug("no data recived any more, stopped");
        //״̬�ı�Ϊ��ʱ����ʱǰ��ȫ���Ͽ�������Ҫ��ʱ��ֱ������
        m_bOver = true;

		if(m_pOrigin)
			m_pOrigin->stop();
        MutexLock lock(&m_csChls);
        for(auto it:m_mapChlEx){
            it.second->stop();
        }
    }

    vector<ClientInfo> CLiveWorker::GetClientInfo()
    {
		vector<ClientInfo> ret;
		if(m_pOrigin)
			ret = m_pOrigin->GetClientInfo();
        MutexLock lock(&m_csChls);
        for(auto chl : m_mapChlEx){
            vector<ClientInfo> tmp = chl.second->GetClientInfo();
            ret.insert(ret.end(), tmp.begin(), tmp.end());
        }
		return ret;
    }

    void CLiveWorker::ReceiveStream(AV_BUFF buff)
    {
        if(buff.eType == AV_TYPE::RTP) {

        } else if(buff.eType == AV_TYPE::H264_NALU) {
            //ԭʼͨ������h264����ת����ȥ
            if(m_pOrigin){
                m_pOrigin->ReceiveStream(buff);
            }
            //��չͨ���������������yuv�ٷ��͹�ȥ
#ifdef USE_FFMPEG
            MutexLock lock(&m_csChls);
            if(!m_mapChlEx.empty()){
                if(nullptr == m_pDecoder) {
                    m_pDecoder = IDecoder::Create(H264DecodeCb,this);
                    for(auto it:m_mapChlEx){
                        it.second->SetDecoder(m_pDecoder);
                    }
                }
                m_pDecoder->Decode(buff);
            } else {
                SAFE_DELETE(m_pDecoder);
            }
#endif
        }
    }

    void CLiveWorker::ReceiveYUV(AV_BUFF buff)
    {
        for (auto it:m_mapChlEx)
        {
            it.second->ReceiveStream(buff);
        }
    }

}
