#pragma once
#include "commonDefine.h"
#include "liveObj.h"
#include "NetStreamMaker.h"

/** ֱ��ʱ�õ�fmp4��ʽ
---------------fmp4-head-----------------------------
ftyp
moov    mvhd
        trak    tkhd
                media   mdhd
                        hdlr
                        minf    head    vmhd
                                        smhd
                                        hmhd
                                        nmhd
                                dinf    dref    url
                                                urn
                                stbl    stsd
                                        stts
                                        stsz
                                        stsc
                                        stss
                                        stco
        trak...
        mvex    trex
--------------fmp4-fragment--------------------------
moof    mfhd
        traf    tfhd
                tfdt
                trun
mdat
moof
mdat...
 */

enum MP4_FRAG_TYPE
{
    MP4_HEAD,
    MP4_FRAG
};

typedef void (*MP4_CB)(MP4_FRAG_TYPE,char*,uint32_t);

class CMP4 : public IAnalyzer
{
public:
    CMP4(CLiveObj* pObj);
    ~CMP4();

    int InputBuffer(char* pBuf, long nLen);

private:
    bool ParseSPS();

    bool MakeHeader();

    bool MakeVideo();

private:
    CNetStreamMaker    *m_pSPS;            // ����SPS
    CNetStreamMaker    *m_pPPS;            // ����PPS
    CNetStreamMaker    *m_pMdat;           // ����h264����
    CNetStreamMaker    *m_pSamplePos;      // ����ÿ��sample��λ�ƣ�trun��sample_composition_time_offset
    uint32_t           m_nSampleNum;       // �����sample��������������sps��pps��nalu����(ÿ��naluΪһ֡)
    CNetStreamMaker    *m_pHeader;         // �������ɵ�MP4ͷ
    CNetStreamMaker    *m_pFragment;       // �������ɵ�MP4����

    uint32_t           m_timestamp;       // ʱ���
    uint32_t           m_tick_gap;        // ��֡��ļ��

    CLiveObj*          m_pObj;

    // SPS����������Ϣ
    uint32_t           m_nWidth;          // ��Ƶ��
    uint32_t           m_nHeight;         // ��Ƶ��
    uint32_t           m_nHorizresolution;// ˮƽdpi
    uint32_t           m_mVertresolution; // ��ֱdpi
    double             m_nfps;            // ��Ƶ֡��

    // ״̬
    bool               m_bMakeHeader;     // ������MP4ͷ
    bool               m_bFirstKey;       // �Ѿ������һ���ؼ�֡
    bool               m_bRun;            // ִ��״̬
    uint32_t           m_nSeq;            // MP4Ƭ�����
};