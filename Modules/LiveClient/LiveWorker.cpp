#include "stdafx.h"
#include "uv.h"
#include "LiveWorker.h"
#include "LiveClient.h"
#include "liveReceiver.h"
#include "LiveChannel.h"
#include "LiveIpc.h"
#include "bnf.h"

namespace LiveClient
{
	extern uv_loop_t      *g_uv_loop;
    extern string          g_strRtpIP;       //< RTP����IP
    extern vector<int>     g_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���
    
    static map<string,CLiveWorker*>  m_workerMap;

    static int GetRtpPort()
    {
        int nRet = -1;
        auto it = g_vecRtpPort.begin();
        if (it != g_vecRtpPort.end()) {
            nRet = *it;
            g_vecRtpPort.erase(it);
        }
        return nRet;
    }

    static void GiveBackRtpPort(int nPort)
    {
        g_vecRtpPort.push_back(nPort);
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

        CLiveWorker* pNew = new CLiveWorker(strCode, rtpPort);
        m_workerMap.insert(make_pair(strCode, pNew));

        LiveIpc::RealPlay(strCode, g_strRtpIP,  rtpPort);
        Log::debug("RealPlay start: %s",strCode.c_str());

        return pNew;
    }

    CLiveWorker* GetLiveWorker(string strCode)
    {
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // �Ѿ�����
            return itFind->second;
        }

        return nullptr;
    }

    bool DelLiveWorker(string strCode)
    {
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

    CLiveWorker::CLiveWorker(string strCode, int rtpPort)
        : m_strCode(strCode)
        , m_bPlayed(false)
        , m_nPlayRes(0)
        , m_nPort(rtpPort)
        , m_nType(0)
        , m_pReceiver(nullptr)
        , m_pOrigin(nullptr)
#ifdef EXTEND_CHANNELS
        , m_pDecoder(nullptr)
#endif
        , m_bStop(false)
        , m_bOver(false)
        , m_stream_type(RTP_STREAM_UNKNOW)
    {
    }

    CLiveWorker::~CLiveWorker()
    {
        if(LiveIpc::StopPlay(m_nPort)) {
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
#ifdef EXTEND_CHANNELS
            MutexLock lock(&m_csChls);
            auto fit = m_mapChlEx.find(c);
            if(fit != m_mapChlEx.end()) {
                fit->second->AddHandle(h, t);
            } else {
                CLiveChannel *nc = new CLiveChannel(c, 640, 480);
                nc->AddHandle(h, t);
                nc->SetDecoder(m_pDecoder);
                m_mapChlEx.insert(make_pair(c, nc));
            }
#else
            // ԭʼͨ��
            if(!m_pOrigin)
                m_pOrigin = new CLiveChannel;
            CHECK_POINT(m_pOrigin);
            m_pOrigin->AddHandle(h, t);
#endif
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
        return m_strPlayInfo;
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
#ifdef EXTEND_CHANNELS
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

#ifdef EXTEND_CHANNELS
    void CLiveWorker::ReceiveYUV(AV_BUFF buff)
    {
        for (auto it:m_mapChlEx)
        {
            it.second->ReceiveStream(buff);
        }
    }
#endif

    void CLiveWorker::PlayAnswer(PlayAnswerList *pal){
        if(!pal->nRet){
            Log::debug("RealPlay ok: %s",pal->strDevCode.c_str());

            //��sdp��������ƵԴip�Ͷ˿�
            m_strPlayInfo = pal->strMark;
            ParseSdp();

            //��������
            m_pReceiver = new CLiveReceiver(m_nPort, this, m_stream_type);
            m_pReceiver->m_strRemoteIP     = m_strServerIP;
            m_pReceiver->m_nRemoteRTPPort  = m_nServerPort;
            m_pReceiver->m_nRemoteRTCPPort = m_nServerPort+1;
            m_pReceiver->StartListen();
        } else {
            Log::error("RealPlay failed: %s %s",pal->strDevCode.c_str(), pal->strMark.c_str());
        }

        m_nPlayRes = pal->nRet;
        m_bPlayed = true;
    }

    bool AsyncPlayCB(PlayAnswerList *pal){
        CLiveWorker* pWorker = GetLiveWorker(pal->strDevCode);
        if(!pWorker){
            Log::error("get live worker failed %s", pal->strDevCode.c_str());
            return false;
        }
        pWorker->PlayAnswer(pal);
        return true;
    }

    void CLiveWorker::ParseSdp()
    {
        //��sdp��������ƵԴip�Ͷ˿�
        bnf_t* sdp_bnf = create_bnf(m_strPlayInfo.c_str(), m_strPlayInfo.size());
        char *sdp_line = NULL;
        char remoteIP[25]={0};
        int remotePort = 0;
        while (bnf_line(sdp_bnf, &sdp_line)) {
            if(sdp_line[0]=='c'){
                sscanf(sdp_line, "c=IN IP4 %[^/\r\n]", remoteIP);
                m_strServerIP = remoteIP;
            } else if(sdp_line[0]=='m') {
                sscanf(sdp_line, "m=video %d ", &remotePort);
                m_nServerPort = remotePort;
            }

            if(sdp_line[0]=='a' && !strncmp(sdp_line, "a=rtpmap:", 9)){
                //a=rtpmap:96 PS/90000
                //a=rtpmap:96 H264/90000
                char tmp[256]={0};
                int num = 0;
                char type[20]={0};
                int bitrate = 0;
                sscanf(sdp_line, "a=rtpmap:%d %[^/]/%d", &num, type, &bitrate);
                if(!strcmp(type, "PS") || !strcmp(type, "MP2P")){
                    m_stream_type = RTP_STREAM_PS;
                }else{
                    m_stream_type = RTP_STREAM_H264;
                }
            }
        }
        destory_bnf(sdp_bnf);
    }
}
