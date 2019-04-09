#include "stdafx.h"
#include "uv.h"
#include "HttpLiveServer.h"
#include "HttpWorker.h"
#include "LiveClient.h"

namespace HttpWsServer
{
    static map<string,CHttpWorker*>  m_workerMapFlv;
    static CriticalSection           m_csFlv;
    static map<string,CHttpWorker*>  m_workerMapMp4;
    static CriticalSection           m_csMp4;
    static map<string,CHttpWorker*>  m_workerMapH264;
    static CriticalSection           m_csH264;
    static map<string,CHttpWorker*>  m_workerMapTs;
    static CriticalSection           m_csTs;

    static void destroy_ring_node(void *_msg)
    {
        LIVE_BUFF *msg = (LIVE_BUFF*)_msg;
        free(msg->pBuff);
        msg->pBuff = NULL;
        msg->nLen = 0;
    }

    //////////////////////////////////////////////////////////////////////////

    CHttpWorker::CHttpWorker(string strCode, HandleType t)
        : m_strCode(strCode)
        , m_nType(0)
        , m_pLive(nullptr)
        , m_type(t)
    {
        memset(&m_stHead, 0, sizeof(m_stHead));
        m_pRing  = lws_ring_create(sizeof(LIVE_BUFF), 100, destroy_ring_node);

        m_pLive = LiveClient::GetWorker(strCode);
        m_pLive->AddHandle(this, t);
    }

    CHttpWorker::~CHttpWorker()
    {
        m_pLive->RemoveHandle(this);

        lws_ring_destroy(m_pRing);
        Log::debug("CHttpWorker release");
    }

    bool CHttpWorker::AddConnect(pss_http_ws_live* pss)
    {
        pss->pss_next = m_pPssList;
        m_pPssList = pss;
        pss->tail = lws_ring_get_oldest_tail(m_pRing);

        return true;
    }

    bool CHttpWorker::DelConnect(pss_http_ws_live* pss)
    {
        lws_ll_fwd_remove(pss_http_ws_live, pss_next, pss, m_pPssList);

        if(m_pPssList == NULL) {
            DelHttpWorker(m_strCode, m_type);
        }
        return true;
    }

    LIVE_BUFF CHttpWorker::GetHeader()
    {
		return m_pLive->GetHeader(m_type);
    }

    LIVE_BUFF CHttpWorker::GetVideo(uint32_t *tail)
    {
        LIVE_BUFF ret = {nullptr,0};
        LIVE_BUFF* tag = (LIVE_BUFF*)lws_ring_get_element(m_pRing, tail);
        if(tag) ret = *tag;

        return ret;
    }

    void CHttpWorker::NextWork(pss_http_ws_live* pss)
    {
        //Log::debug("this work tail:%d\r\n", pss->tail);
        lws_ring_consume_and_update_oldest_tail(
            m_pRing,	          /* lws_ring object */
            pss_http_ws_live, /* type of objects with tails */
            &pss->tail,	      /* tail of guy doing the consuming */
            1,		          /* number of payload objects being consumed */
            m_pPssList,	      /* head of list of objects with tails */
            tail,		      /* member name of tail in objects with tails */
            pss_next	      /* member name of next object in objects with tails */
            );
        //Log::debug("next work tail:%d\r\n", pss->tail);

        /* more to do for us? */
        if (lws_ring_get_element(m_pRing, &pss->tail))
            /* come back as soon as we can write more */
                lws_callback_on_writable(pss->wsi);
    }

    void CHttpWorker::push_video_stream(char* pBuff, int nLen)
    {
        int n = (int)lws_ring_get_count_free_elements(m_pRing);
        if (!n) {
            cull_lagging_clients();
            n = (int)lws_ring_get_count_free_elements(m_pRing);
        }
        Log::debug("ring free space %d\n", n);
        if (!n)
            return;

        // �����ݱ�����ring buff
        char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
        memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
        LIVE_BUFF newTag = {pSaveBuff, nLen};
        if (!lws_ring_insert(m_pRing, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //�����в������ӷ�������
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pPssList) {
            lws_callback_on_writable((*ppss)->wsi);
        } lws_end_foreach_llp(ppss, pss_next);
    }

    void CHttpWorker::stop()
    {
        //��ƵԴû�����ݲ���ʱ
        Log::debug("no data recived any more, stopped");

        //�Ͽ����пͻ�������
        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pPssList, pss_next) {
            lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        } lws_end_foreach_llp_safe(ppss);
    }

    string CHttpWorker::get_clients_info()
    {
        stringstream ss;
        lws_start_foreach_llp(pss_http_ws_live **, ppss, m_pPssList) {
            ss << "{\"DeviceID\":\"" << m_strCode << "\",\"Connect\":\"";
            if((*ppss)->isWs){
                ss << "Web Socket";
            } else {
                ss << "Http";
            }
            ss << "\",\"Media\":\"";
            if(m_type == HandleType::flv_handle)
                ss << "flv";
            else if(m_type == HandleType::fmp4_handle)
                ss << "mp4";
            else if(m_type == HandleType::h264_handle)
                ss << "h264";
            else if(m_type == HandleType::ts_handle)
                ss << "hls";
            else ss << "unknown";
            ss << "\",\"ClientIP\":\"" 
                << (*ppss)->clientIP << "\"},";
        } lws_end_foreach_llp(ppss, pss_next);
        return ss.str();
    }

    void CHttpWorker::cull_lagging_clients()
    {
        uint32_t oldest_tail = lws_ring_get_oldest_tail(m_pRing);
        pss_http_ws_live *old_pss = NULL;
        int most = 0, before = lws_ring_get_count_waiting_elements(m_pRing, &oldest_tail), m;

        lws_start_foreach_llp_safe(pss_http_ws_live **, ppss, m_pPssList, pss_next) {
            if ((*ppss)->tail == oldest_tail) {
                //���ӳ�ʱ
                old_pss = *ppss;
                Log::debug("Killing lagging client %p", (*ppss)->wsi);
                lws_set_timeout((*ppss)->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);
                (*ppss)->culled = 1;
                lws_ll_fwd_remove(pss_http_ws_live, pss_next, (*ppss), m_pPssList);
                continue;
            } else {
                m = lws_ring_get_count_waiting_elements(m_pRing, &((*ppss)->tail));
                if (m > most)
                    most = m;
            }
        } lws_end_foreach_llp_safe(ppss);

        if (!old_pss)
            return;

        lws_ring_consume_and_update_oldest_tail(m_pRing,
            pss_http_ws_live, &old_pss->tail, before - most,
            m_pPssList, tail, pss_next);

        Log::debug("shrunk ring from %d to %d", before, most);
    }

    //////////////////////////////////////////////////////////////////////////

    CHttpWorker* CreatHttpWorker(string strCode, HandleType t)
    {
        Log::debug("CreatHttpWorker begin");
        if(t == HandleType::flv_handle){
            CHttpWorker* pNew = new CHttpWorker(strCode, t);
            MutexLock lock(&m_csFlv);
            m_workerMapFlv.insert(make_pair(strCode, pNew));
            return pNew;
        } else if(t == HandleType::fmp4_handle) {
            CHttpWorker* pNew = new CHttpWorker(strCode, t);
            MutexLock lock(&m_csMp4);
            m_workerMapMp4.insert(make_pair(strCode, pNew));
            return pNew;
        } else if(t == HandleType::h264_handle) {
            CHttpWorker* pNew = new CHttpWorker(strCode, t);
            MutexLock lock(&m_csH264);
            m_workerMapH264.insert(make_pair(strCode, pNew));
            return pNew;
        } else if(t == HandleType::ts_handle) {
            CHttpWorker* pNew = new CHttpWorker(strCode, t);
            MutexLock lock(&m_csTs);
            m_workerMapTs.insert(make_pair(strCode, pNew));
            return pNew;
        }
        return nullptr;
    }

    CHttpWorker* GetHttpWorker(string strCode, HandleType t)
    {
        //Log::debug("GetWorker begin");

        if(t == flv_handle) {
            MutexLock lock(&m_csFlv);
            auto itFind = m_workerMapFlv.find(strCode);
            if (itFind != m_workerMapFlv.end()) {
                return itFind->second;
            }
        } else if(t == fmp4_handle) {
            MutexLock lock(&m_csMp4);
            auto itFind = m_workerMapMp4.find(strCode);
            if (itFind != m_workerMapMp4.end()) {
                return itFind->second;
            }
        } else if(t == h264_handle) {
            MutexLock lock(&m_csH264);
            auto itFind = m_workerMapH264.find(strCode);
            if (itFind != m_workerMapH264.end()) {
                return itFind->second;
            }
        } else if(t == ts_handle) {
            MutexLock lock(&m_csTs);
            auto itFind = m_workerMapTs.find(strCode);
            if (itFind != m_workerMapTs.end()) {
                return itFind->second;
            }
        }

        //Log::error("GetWorker failed: %s",strCode.c_str());
        return nullptr;
    }

    bool DelHttpWorker(string strCode, HandleType t)
    {
        if(t == flv_handle) {
            MutexLock lock(&m_csFlv);
            auto itFind = m_workerMapFlv.find(strCode);
            if (itFind != m_workerMapFlv.end()) {
                SAFE_DELETE(itFind->second);
                m_workerMapFlv.erase(itFind);
                return true;
            }
        } else if(t == fmp4_handle) {
            MutexLock lock(&m_csMp4);
            auto itFind = m_workerMapMp4.find(strCode);
            if (itFind != m_workerMapMp4.end()) {
                SAFE_DELETE(itFind->second);
                m_workerMapMp4.erase(itFind);
                return true;
            }
        } else if(t == h264_handle) {
            MutexLock lock(&m_csH264);
            auto itFind = m_workerMapH264.find(strCode);
            if (itFind != m_workerMapH264.end()) {
                SAFE_DELETE(itFind->second);
                m_workerMapH264.erase(itFind);
                return true;
            }
        } else if(t == ts_handle) {
            MutexLock lock(&m_csTs);
            auto itFind = m_workerMapTs.find(strCode);
            if (itFind != m_workerMapTs.end()) {
                SAFE_DELETE(itFind->second);
                m_workerMapTs.erase(itFind);
                return true;
            }
        }

        Log::error("dosn't find worker object");
        return false;
    }

}