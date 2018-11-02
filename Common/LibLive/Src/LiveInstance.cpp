#include "stdafx.h"
#include "uv.h"
#include "LiveInstance.h"
#include "Rtp.h"
#include "Ps.h"
#include "Pes.h"
#include "ES.h"
#include "TS.h"
#include "Flv.h"

IlibLive* IlibLive::CreateObj()
{
    return new CLiveInstance;
}

bool IlibLive::MakeFlvHeader(char** ppBuff, int* pLen)
{
    return CFlv::MakeHeader(ppBuff, pLen);
}

static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

static void after_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
{
    Log::debug("after_read thread ID : %d", GetCurrentThreadId());
    if(nread < 0){
        Log::error("read error: %s",uv_strerror(nread));
        free(buf->base);
    }
    CLiveInstance* pLive = (CLiveInstance*)handle->data;
    pLive->RtpRecv(buf->base, nread);
}

static void timer_cb(uv_timer_t* handle)
{
    CLiveInstance* pLive = (CLiveInstance*)handle->data;
    pLive->RtpOverTime();
}

CLiveInstance::CLiveInstance(void)
    : m_nCatchPacketNum(100)
    , m_pRtpParser(nullptr)
    , m_pPsParser(nullptr)
    , m_pPesParser(nullptr)
    , m_pEsParser(nullptr)
    , m_pTs(nullptr)
    , m_pFlv(nullptr)
    , m_pCallBack(nullptr)
    , m_pRtpBuff(nullptr)
    , m_nRtpLen(0)
    , m_pPsBuff(nullptr)
    , m_nPsLen(0)
    , m_pPesBuff(nullptr)
    , m_nPesLen(0)
    , m_pEsBuff(nullptr)
    , m_nEsLen(0)
    , m_pNaluBuff(nullptr)
    , m_nNaluLen(0)
    , m_pts(0)
    , m_dts(0)
    , m_nalu_type(0)
{
    m_pRtpParser     = new CRtp(this);
    m_pPsParser      = new CPs(this);
    m_pPesParser     = new CPes(this);
    m_pEsParser      = new CES(this);
    m_pTs            = new CTS;
    m_pFlv           = new CFlv;

    // ���ûص�
    CFlv* flv = (CFlv*)m_pFlv;
    flv->SetCallBack([&](flv_tag_type tagType,char* pFlvTag, uint32_t nTagLen){
        m_pCallBack->push_flv_frame(tagType, pFlvTag, nTagLen);
    });
    CTS* ts = (CTS*)m_pTs;
    ts->SetCallBack([&](char* pTsBuff, uint32_t nTsLen){
        m_pCallBack->push_ts_stream(pTsBuff, nTsLen);
    });
}

CLiveInstance::~CLiveInstance(void)
{
    m_pCallBack = nullptr;
    int ret = uv_udp_recv_stop(&m_uvRtpSocket);
    if(ret < 0) {
        Log::error("stop rtp recv port:%d err: %s", m_nLocalRTPPort, uv_strerror(ret));
    }
    ret = uv_timer_stop(&m_uvTimeOver);
    if(ret < 0) {
        Log::error("stop timer error: %s",uv_strerror(ret));
    }

    SAFE_DELETE(m_pRtpParser);
    SAFE_DELETE(m_pPsParser);
    SAFE_DELETE(m_pPesParser);
    SAFE_DELETE(m_pEsParser);
    SAFE_DELETE(m_pTs);
    SAFE_DELETE(m_pFlv);
    Sleep(2000);
}

void CLiveInstance::StartListen()
{
    // ����udp����
    int ret = uv_udp_init(uv_default_loop(), &m_uvRtpSocket);
    if(ret < 0) {
        Log::error("udp init error: %s", uv_strerror(ret));
        return;
    }

    struct sockaddr_in addr;
    ret = uv_ip4_addr("0.0.0.0", m_nLocalRTPPort, &addr);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return ;
    }

    ret = uv_udp_bind(&m_uvRtpSocket, (struct sockaddr*)&addr, 0);
    if(ret < 0) {
        Log::error("tcp bind err: %s",  uv_strerror(ret));
        return;
    }

    m_uvRtpSocket.data = (void*)this;
    uv_udp_recv_start(&m_uvRtpSocket, echo_alloc, after_read);

    //����udp���ճ�ʱ�ж�
    ret = uv_timer_init(uv_default_loop(), &m_uvTimeOver);
    if(ret < 0) {
        Log::error("timer init error: %s", uv_strerror(ret));
        return;
    }

    m_uvTimeOver.data = (void*)this;
    ret = uv_timer_start(&m_uvTimeOver, timer_cb,30000, 30000);
    if(ret < 0) {
        Log::error("timer start error: %s", uv_strerror(ret));
        return;
    }
}

void CLiveInstance::RtpRecv(char* pBuff, long nLen)
{
    int ret = uv_timer_again(&m_uvTimeOver);
    if(ret < 0) {
        Log::error("timer again error: %s", uv_strerror(ret));
        return;
    }
    CRtp* rtpAnalyzer = (CRtp*)m_pRtpParser;
    rtpAnalyzer->InputBuffer(pBuff, nLen);
}

void CLiveInstance::RtpOverTime()
{
    Log::debug("OverTimeThread thread ID : %d", GetCurrentThreadId());
    time_t nowTime = time(NULL);
    // rtp���ճ�ʱ
    if(nullptr != m_pCallBack)
    {
        m_pCallBack->stop();
    }
}

void CLiveInstance::RTPParseCb(char* pBuff, long nLen)
{
    //Log::debug("CRTSPInterface::RTPParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(pBuff)
    CPs* pPsParser = (CPs*)m_pPsParser;
    CHECK_POINT_VOID(pPsParser)
    m_pPsBuff = pBuff;
    m_nPsLen  = nLen;
    pPsParser->InputBuffer(pBuff, nLen);
}

void CLiveInstance::PSParseCb(char* pBuff, long nLen)
{
    //Log::debug("CRTSPInterface::PSParseCb nlen:%ld", nLen);
    CHECK_POINT_VOID(pBuff)
    CPes* pPesParser = (CPes*)m_pPesParser;
    CHECK_POINT_VOID(pPesParser)
    m_pPesBuff = pBuff;
    m_nPesLen  = nLen;
    pPesParser->InputBuffer(pBuff, nLen);
}

void CLiveInstance::PESParseCb(char* pBuff, long nLen, uint64_t pts, uint64_t dts)
{
    //Log::debug("CRTSPInterface::PESParseCb nlen:%ld,pts:%lld,dts:%lld", nLen,pts,dts);
    CHECK_POINT_VOID(pBuff)
    CES* pEsParser = (CES*)m_pEsParser;
    CHECK_POINT_VOID(pEsParser)
    m_pEsBuff = pBuff;
    m_nEsLen  = nLen;
    m_pts = pts;
    m_dts = dts;
	pEsParser->InputBuffer(pBuff, nLen);
}

void CLiveInstance::ESParseCb(char* pBuff, long nLen, uint8_t nNalType)
{
    //nal_unit_header4* nalu = (nal_unit_header4*)pBuff;
    //Log::debug("CLiveInstance::ESParseCb nlen:%ld, buff:%02X %02X %02X %02X %02X", nLen,pBuff[0],pBuff[1],pBuff[2],pBuff[3],pBuff[4]);
    CHECK_POINT_VOID(pBuff)
    m_pNaluBuff = pBuff;
    m_nNaluLen  = nLen;
    m_nalu_type = nNalType;

    if(nullptr != m_pCallBack)
    {
        //��Ҫ�ص�Flv
        if(m_pCallBack->m_bFlv && nullptr != m_pFlv)
        {
            CFlv* flv = (CFlv*)m_pFlv;
            flv->InputBuffer(pBuff,nLen);
        }

        //��Ҫ�ص�H264
        if(nullptr != m_pCallBack && m_pCallBack->m_bH264)
        {
            m_pCallBack->push_h264_stream((NalType)nNalType, pBuff, nLen);
        }

        //��Ҫ�ص�TS
        if(m_pCallBack->m_bTs && nullptr != m_pTs)
        {
            CTS* ts = (CTS*)m_pTs;
            ts->SetParam(nNalType, m_pts);
            ts->InputBuffer(m_pPesBuff, m_nPesLen);
        }
    }
}