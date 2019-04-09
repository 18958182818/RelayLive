#include "stdafx.h"
#include "HttpLiveServer.h"
#include "HttpWorker.h"
//����ģ��
#include "LiveClient.h"
#include "uvIpc.h"
#include "uv.h"

namespace HttpWsServer
{
	extern uv_loop_t *g_uv_loop;

    static map<string,CHttpWorker*>  m_workerMap;
    static CriticalSection           m_cs;

    static void destroy_ring_node(void *_msg)
    {
        LIVE_BUFF *msg = (LIVE_BUFF*)_msg;
        free(msg->pBuff);
        msg->pBuff = NULL;
        msg->nLen = 0;
    }

    /** ��ʱ���ٶ�ʱ����loop���Ƴ���� */
    static void stop_timer_close_cb(uv_handle_t* handle) {
        CHttpWorker* live = (CHttpWorker*)handle->data;
        if (live->m_bStop){
            live->Clear2Stop();
        } else {
            Log::debug("new client comed, and will not close live stream");
        }
    }

    /** �ͻ���ȫ���Ͽ�����ʱ�Ͽ�Դ�Ķ�ʱ�� */
	static void stop_timer_cb(uv_timer_t* handle) {
		CHttpWorker* live = (CHttpWorker*)handle->data;
		int ret = uv_timer_stop(handle);
		if(ret < 0) {
			Log::error("timer stop error:%s",uv_strerror(ret));
        }
        live->m_bStop = true;
        uv_close((uv_handle_t*)handle, stop_timer_close_cb);
	}

    /** CLiveWorker������ɾ��m_pLive�ȽϺ�ʱ��������event loop�����ʹ���̡߳� */
    static void live_worker_destory_thread(void* arg) {
        CHttpWorker* live = (CHttpWorker*)arg;
        SAFE_DELETE(live);
    }

    //////////////////////////////////////////////////////////////////////////

    CHttpWorker::CHttpWorker(string strCode)
        : m_strCode(strCode)
        , m_nType(0)
        , m_pLive(nullptr)
        , m_bStop(false)
        , m_bOver(false)
    {
        memset(&m_stFlvHead, 0, sizeof(m_stFlvHead));
        memset(&m_stMP4Head, 0, sizeof(m_stMP4Head));
        m_pFlvRing  = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);
        m_pH264Ring = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);
        m_pMP4Ring  = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);

		liblive_option opt = {m_nPort, m_nRtpCatchPacketNum, m_nRtpStreamType};
        m_pLive = IlibLive::CreateObj(opt);
        m_pLive->SetCallback(this);
        m_pLive->StartListen();
    }

    CHttpWorker::~CHttpWorker()
    {
        //if(m_nType == 0) {
        //    if (!SipInstance::StopPlay(m_strCode)) {
        //        Log::error("stop play failed");
        //    }
        //} else {
        //    if (!SipInstance::StopRecordPlay(m_strCode, "")) {
        //        Log::error("stop play failed");
        //    }
        //}
        if(stop_play(m_strCode)) {
            Log::error("stop play failed");
        }
        SAFE_DELETE(m_pLive);
        lws_ring_destroy(m_pFlvRing);
        lws_ring_destroy(m_pH264Ring);
        lws_ring_destroy(m_pMP4Ring);
        Log::debug("CLiveWorker release");
    }

    bool CHttpWorker::AddConnect(pss_http_ws_live* pss)
    {
        if(pss->media_type == media_flv) {
            m_bFlv = true;
            pss->pss_next = m_pFlvPssList;
            m_pFlvPssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_pFlvRing);
        } else if (pss->media_type == media_h264) {
            m_bH264 = true;
            pss->pss_next = m_pH264PssList;
            m_pH264PssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_pH264Ring);
        } else if (pss->media_type == media_mp4) {
            m_bMp4 = true;
            pss->pss_next = m_pMP4PssList;
            m_pMP4PssList = pss;
            pss->tail = lws_ring_get_oldest_tail(m_pMP4Ring);
        } else {
            return false;
        }

        //����ոտ����˽�����ʱ������Ҫ����ر�
        if(uv_is_active((const uv_handle_t*)&m_uvTimerStop)) {
            uv_timer_stop(&m_uvTimerStop);
            uv_close((uv_handle_t*)&m_uvTimerStop, stop_timer_close_cb);
        }
        return true;
    }

    bool CHttpWorker::DelConnect(pss_http_ws_live* pss)
    {
        if(pss->media_type == media_flv) {
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pFlvPssList);
            if(nullptr == m_pFlvPssList) m_bFlv = false;
        } else if(pss->media_type == media_h264){
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pH264PssList);
            if (nullptr == m_pH264PssList) m_bH264 = false;
        } else if(pss->media_type == media_mp4) {
            lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pMP4PssList);
            if (nullptr == m_pMP4PssList) m_bMp4 = false;
        }

        if(m_pFlvPssList == NULL && m_pH264PssList == NULL && m_pMP4PssList == NULL) {
            if(m_bOver) {
                // ��ƵԴû�����ݣ���ʱ������Ҫ��ʱ
                Clear2Stop();
            } else {
                // ��ƵԴ��Ȼ���ӣ���ʱ20�������ٶ����Ա��ʱ�������������ܿ��ٲ���
                uv_timer_init(g_uv_loop, &m_uvTimerStop);
                m_uvTimerStop.data = this;
                uv_timer_start(&m_uvTimerStop, stop_timer_cb, 20000, 0);
            }
        }
        return true;
    }

	void CHttpWorker::Clear2Stop() {
		if(m_pFlvPssList == NULL && m_pH264PssList == NULL && m_pMP4PssList == NULL) {
			Log::debug("need close live stream");
            //���ȴ�map�����߶���
            DelHttpWorker(m_strCode);
		}
	}

    void CHttpWorker::push_flv_frame(FLV_FRAG_TYPE eType, char* pBuff, int nLen)
    {
        if (eType == FLV_HEAD) {
            m_stFlvHead.pBuff = pBuff;
            m_stFlvHead.nLen = nLen;
            Log::debug("flv head ok");
        } else {
            //�ڴ����ݱ�����ring-buff
            int n = (int)lws_ring_get_count_free_elements(m_pFlvRing);
            if (!n) {
                cull_lagging_clients(media_flv);
                n = (int)lws_ring_get_count_free_elements(m_pFlvRing);
            }
            Log::debug("LWS_CALLBACK_RECEIVE: free space %d\n", n);
            if (!n)
                return;

            // �����ݱ�����ring buff
            char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
            memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
            LIVE_BUFF newTag = {pSaveBuff, nLen};
            if (!lws_ring_insert(m_pFlvRing, &newTag, 1)) {
                destroy_ring_node(&newTag);
                Log::error("dropping!");
                return;
            }

            //�����в������ӷ�������
            lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pFlvPssList) {
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_next);
        }    
    }

    void CHttpWorker::push_h264_stream(char* pBuff, int nLen)
    {
        int n = (int)lws_ring_get_count_free_elements(m_pH264Ring);
        if (!n) {
            cull_lagging_clients(media_h264);
            n = (int)lws_ring_get_count_free_elements(m_pH264Ring);
        }
        Log::debug("h264 ring free space %d\n", n);
        if (!n)
            return;

        // �����ݱ�����ring buff
        char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
        memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
        LIVE_BUFF newTag = {pSaveBuff, nLen};
        if (!lws_ring_insert(m_pH264Ring, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //�����в������ӷ�������
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pH264PssList) {
            lws_callback_on_writable((*ppss)->wsi);
        } lws_end_foreach_llp(ppss, pss_next);
    }

    void CHttpWorker::push_ts_stream(char* pBuff, int nLen)
    {
    }

    void CHttpWorker::push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nLen)
    {
        if(eType == MP4_HEAD) {
            m_stMP4Head.pBuff = pBuff;
            m_stMP4Head.nLen = nLen;
            Log::debug("MP4 Head ok");
        } else {
            int n = (int)lws_ring_get_count_free_elements(m_pMP4Ring);
            if (!n) {
                cull_lagging_clients(media_mp4);
                n = (int)lws_ring_get_count_free_elements(m_pMP4Ring);
            }
            Log::debug("mp4 ring free space %d\n", n);
            if (!n)
                return;

            // �����ݱ�����ring buff
            char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
            memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
            LIVE_BUFF newTag = {pSaveBuff, nLen};
            if (!lws_ring_insert(m_pMP4Ring, &newTag, 1)) {
                destroy_ring_node(&newTag);
                Log::error("dropping!");
                return;
            }

            //�����в������ӷ�������
            lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pMP4PssList) {
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_next);
        }
    }

    void CHttpWorker::stop()
    {
        //��ƵԴû�����ݲ���ʱ
        Log::debug("no data recived any more, stopped");
        //״̬�ı�Ϊ��ʱ����ʱǰ��ȫ���Ͽ�������Ҫ��ʱ��ֱ������
        m_bOver = true;

        //�Ͽ����пͻ�������
        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pFlvPssList, pss_next) {
            lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        } lws_end_foreach_llp_safe(ppss);
        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pH264PssList, pss_next) {
            lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        } lws_end_foreach_llp_safe(ppss);
        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pMP4PssList, pss_next) {
            lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        } lws_end_foreach_llp_safe(ppss);
    }

    LIVE_BUFF CHttpWorker::GetFlvHeader()
    {
        return m_stFlvHead;
    }

    LIVE_BUFF CHttpWorker::GetFlvVideo(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pFlvRing, tail);
        if(tag) ret = *tag;

        return ret;
    }

    LIVE_BUFF CHttpWorker::GetH264Video(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pH264Ring, tail);
        if(tag) ret = *tag;

        return ret;
    }

    LIVE_BUFF CHttpWorker::GetMp4Header()
    {
        return m_stMP4Head;
    }

    LIVE_BUFF CHttpWorker::GetMp4Video(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pMP4Ring, tail);
        if(tag) ret = *tag;

        return ret;
    }

    void CHttpWorker::NextWork(pss_http_ws_live* pss)
    {
        struct lws_ring *ring;
        pss_http_ws_live* pssList;
        if(pss->media_type == media_flv) {
            ring = m_pFlvRing;
            pssList = m_pFlvPssList;
        } else if(pss->media_type == media_h264){
            ring = m_pH264Ring;
            pssList = m_pH264PssList;
        } else if (pss->media_type == media_mp4) {
            ring = m_pMP4Ring;
            pssList = m_pMP4PssList;
        } else {
            return;
        }

        //Log::debug("this work tail:%d\r\n", pss->tail);
        lws_ring_consume_and_update_oldest_tail(
            ring,	          /* lws_ring object */
            pss_http_ws_live, /* type of objects with tails */
            &pss->tail,	      /* tail of guy doing the consuming */
            1,		          /* number of payload objects being consumed */
            pssList,	      /* head of list of objects with tails */
            tail,		      /* member name of tail in objects with tails */
            pss_next	      /* member name of next object in objects with tails */
            );
        //Log::debug("next work tail:%d\r\n", pss->tail);

        /* more to do for us? */
        if (lws_ring_get_element(ring, &pss->tail))
            /* come back as soon as we can write more */
                lws_callback_on_writable(pss->wsi);
    }

    string CHttpWorker::GetClientInfo()
    {
        stringstream ss;
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pFlvPssList) {
            ss << "{\"DeviceID\":\"" << m_strCode << "\",\"Connect\":\"";
            if((*ppss)->isWs){
                ss << "web socket";
            } else {
                ss << "Http";
            }
            ss << "\",\"Media\":\"flv\",\"ClientIP\":\"" 
                << (*ppss)->clientIP << "\"},";
        } lws_end_foreach_llp(ppss, pss_next);

        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pH264PssList) {
            ss << "{\"DeviceID\":\"" << m_strCode << "\",\"Connect\":\"";
            if((*ppss)->isWs){
                ss << "web socket";
            } else {
                ss << "Http";
            }
            ss << "\",\"Media\":\"h264\",\"ClientIP\":\"" 
                << (*ppss)->clientIP << "\"},";
        } lws_end_foreach_llp(ppss, pss_next);

        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pMP4PssList) {
            ss << "{\"DeviceID\":\"" << m_strCode << "\",\"Connect\":\"";
            if((*ppss)->isWs){
                ss << "web socket";
            } else {
                ss << "Http";
            }
            ss << "\",\"Media\":\"mp4\",\"ClientIP\":\"" 
                << (*ppss)->clientIP << "\"},";
        } lws_end_foreach_llp(ppss, pss_next);

        return ss.str();
    }

    void CHttpWorker::cull_lagging_clients(MediaType type)
    {
        struct lws_ring *ring;
        pss_http_ws_live* pssList;
        if(type == media_flv) {
            ring = m_pFlvRing;
            pssList = m_pFlvPssList;
        } else {
            ring = m_pH264Ring;
            pssList = m_pH264PssList;
        }

        uint32_t oldest_tail = lws_ring_get_oldest_tail(ring);
        pss_http_ws_live *old_pss = NULL;
        int most = 0, before = lws_ring_get_count_waiting_elements(ring, &oldest_tail), m;

        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, pssList, pss_next) {
            if ((*ppss)->tail == oldest_tail) {
                //���ӳ�ʱ
                old_pss = *ppss;
                Log::debug("Killing lagging client %p", (*ppss)->wsi);
                lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);
                (*ppss)->culled = 1;
                lws_ll_fwd_remove(pss_http_ws_live, pss_next, (*ppss), pssList);
                continue;
            } else {
                m = lws_ring_get_count_waiting_elements(ring, &((*ppss)->tail));
                if (m > most)
                    most = m;
            }
        } lws_end_foreach_llp_safe(ppss);

        if (!old_pss)
            return;

        lws_ring_consume_and_update_oldest_tail(ring,
            pss_http_ws_live, &old_pss->tail, before - most,
            pssList, tail, pss_next);

        Log::debug("shrunk ring from %d to %d", before, most);
    }

    //////////////////////////////////////////////////////////////////////////

    void ipc_init(){
        /** ���̼�ͨ�� */
        int ret = uv_ipc_client(&h, "relay_live", NULL, "liveDest", on_ipc_recv, NULL);
        if(ret < 0) {
            printf("ipc server err: %s\n", uv_ipc_strerr(ret));
        }
    }

    CHttpWorker* CreatHttpWorker(string strCode)
    {
        Log::debug("CreatHttpWorker begin");

        CHttpWorker* pNew = new CHttpWorker(strCode);

        MutexLock lock(&m_cs);
        m_workerMap.insert(make_pair(strCode, pNew));

        return pNew;
    }

    CHttpWorker* GetHttpWorker(string strCode)
    {
        //Log::debug("GetWorker begin");

        MutexLock lock(&m_cs);
        auto itFind = m_workerMap.find(strCode);
        if (itFind != m_workerMap.end())
        {
            // �Ѿ�����
            return itFind->second;
        }

        //Log::error("GetWorker failed: %s",strCode.c_str());
        return nullptr;
    }

    bool DelHttpWorker(string strCode)
    {
        MutexLock lock(&m_cs);
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

    string GetClientsInfo() 
    {
        MutexLock lock(&m_cs);
        auto it = m_workerMap.begin();
        auto end = m_workerMap.end();
        string strResJson = "{\"root\":[";
        for (;it != end; ++it)
        {
            strResJson += it->second->GetClientInfo();
        }
        strResJson = StringHandle::StringTrimRight(strResJson,',');
        strResJson += "]}";
        return strResJson;
    }
}