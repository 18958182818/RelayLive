#include "stdafx.h"
#include "Ps.h"
#include "Pes.h"


CPs::CPs(CLiveInstance* pObj)
    : m_pObj(pObj)
{
}


CPs::~CPs(void)
{
}

int CPs::InputBuffer(char* pBuf, long nLen)
{
    long nHeadLen = 0;
    if(0 != ParseHeader(pBuf, nLen, nHeadLen))
    {
        Log::error("CPsAnalyzer::InsertPacket ParseHeader failed");
        return -1;
    }
    if (nLen < nHeadLen)
    {
        Log::error("CPsAnalyzer::InsertPacket nLen:%ld,nHeadLen:%ld",nLen,nHeadLen);
        return -1;
    }
    //Log::debug("CPs::InputBuffer HeadLen:%ld",nHeadLen);

    if (0 != ParsePES(pBuf+nHeadLen, nLen-nHeadLen))
    {
        Log::error("CPsAnalyzer::InsertPacket ParsePES failed");
        return -1;
    }
    return 0;
}

int CPs::ParseHeader(char* pBuf, long nLen, long& nHeadLen)
{
    int nPos = 0;
    ps_header_t* ps = (ps_header_t*)pBuf;
    if(!is_ps_header(ps))
    {
        Log::error("CPsAnalyzer::ParseHeader this is not a ps frame");
        return -1;
    }
    if(ps->fix_bit != 01)
    {
        Log::error("CPsAnalyzer::ParseHeader fix bit is:%d",ps->fix_bit);
        return -1;
    }
    int32_t nSCR = ps->system_clock_reference_base1;
    nSCR <<= 2;
    nSCR += ps->system_clock_reference_base21;
    nSCR <<= 8;
    nSCR += ps->system_clock_reference_base22;
    nSCR <<= 5;
    nSCR += ps->system_clock_reference_base23;
    nSCR <<= 2;
    nSCR += ps->system_clock_reference_base31;
    nSCR <<= 8;
    nSCR += ps->system_clock_reference_base32;
    nSCR <<= 5;
    nSCR += ps->system_clock_reference_base33;

    int32_t nSRCE = ps->system_clock_reference_extension1;
    nSRCE <<= 7;
    nSRCE += ps->system_clock_reference_extension2;

    int32_t nMuxRate = ps->program_mux_rate1;
    nMuxRate <<= 8;
    nMuxRate += ps->program_mux_rate2;
    nMuxRate <<= 6;
    nMuxRate += ps->program_mux_rate3;

    int32_t nStuffLen = ps->pack_stuffing_length;

    //Log::debug("system_clock_reference:%d, system_clock_reference_extension:%d, program_mux_rate:%d,pack_stuffing_length:%d",nSCR,nSRCE,nMuxRate,nStuffLen);

    // ����psͷ
    nPos += (14 + nStuffLen);
    if(nPos + 18 > nLen)
    {
        //���ݳ��Ȳ���
        Log::debug("CPsAnalyzer::ParseHeader no system header");
        return -1;
    }
    sh_header_t* sh = (sh_header_t*)(pBuf+nPos);
    // �ж��Ƿ����system header
    if (is_sh_header(sh))
    {
        // ����ϵͳͷ
        int32_t shLen = sh->header_length[0];
        shLen <<= 8;
        shLen += sh->header_length[1];
        //Log::debug("is_sh_header shLen:%d",shLen);
        nPos += (6 + shLen);
        if(nPos + 36 > nLen)
        {
            //���ݳ��Ȳ���
            Log::error("CPsAnalyzer::ParseHeader no program stream map");
            return -1;
        }
        psm_header_t* psm = (psm_header_t*)(pBuf+nPos);
        // �ж��Ƿ����program stream map;
        if(is_psm_header(psm))
        {
            int32_t psmLen = psm->program_stream_map_length[0];
            psmLen <<= 8;
            psmLen += psm->program_stream_map_length[1];
            //Log::debug("is_psm_header psmLen:%d,stream_type:0x%x",psmLen, psm->stream_type);
            nPos += (6 + psmLen);
        }
    }
    nHeadLen = nPos;
    return 0;
}

int CPs::ParsePES(char* pBuf, long nLen)
{
    long nPos = 0;
    //for (; nPos<nLen-6;)
    {
        pes_header_t* pes = (pes_header_t*)(pBuf + nPos);
        if (!is_pes_header(pes))
        {
            Log::error("CPsAnalyzer::ParsePES is not pes header");
            nPos++;
            //continue;
        }
        int32_t pesLen =  pes->PES_packet_length[0];
        pesLen <<= 8;
        pesLen += pes->PES_packet_length[1];
        //Log::debug("CPsAnalyzer::ParsePES this pes len:%d(+head 6), totle:%d",pesLen,nLen);

        // �ص�����PES��
        if (m_pObj != nullptr)
        {
            m_pObj->PSParseCb((char*)pes,pesLen+6);
        }

        nPos += (6+pesLen);
    }
    return 0;
}