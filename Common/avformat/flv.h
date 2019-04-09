#pragma once
#include "liveObj.h"
#include "NetStreamMaker.h"

enum flv_tag_type
{
    callback_flv_header = 0,
    callback_script_tag,
    callback_video_spspps_tag,
    callback_key_video_tag,
    callback_video_tag
};
enum FLV_FRAG_TYPE
{
    FLV_HEAD,
    FLV_FRAG
};

typedef void (*FLV_CALLBACK)(FLV_FRAG_TYPE, char*, int, void*);

class CFlvStreamMaker : public CNetStreamMaker
{
public:
    void append_amf_string(const char *str );
    void append_amf_double(double d );
} ;

class CFlv
{
public:
    CFlv(void* handle);
    ~CFlv(void);

    int InputBuffer(NalType eType, char* pBuf, uint32_t nLen);

    void SetSps(uint32_t nWidth, uint32_t nHeight, double fFps);

private:
    /**
     * ����flv�ļ�ͷ��Ϣ������
     * @param ppBuff ���flvͷ����
     * @param pLen ���flvͷ����
     */
    bool MakeHeader();

    /**
     * ����һ����Ƶ�ϲ�����
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

private:
    CFlvStreamMaker*        m_pSPS;            // ����SPS���ݵĻ���
    CFlvStreamMaker*        m_pPPS;            // ����PPS���ݵĻ���
    CFlvStreamMaker*        m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
    CFlvStreamMaker*        m_pHeader;         // flvͷ����
    CFlvStreamMaker*        m_pData;           // ������ɵ�flv����

    uint32_t           m_timestamp;       // ʱ���
    uint32_t           m_tick_gap;        // ��֡��ļ��

    void*             m_hUser;                  // �ص��������
    FLV_CALLBACK      m_fCB;

    // SPS����������Ϣ
    uint32_t           m_nWidth;          // ��Ƶ��
    uint32_t           m_nHeight;         // ��Ƶ��
    double             m_nfps;            // ��Ƶ֡��

    // ״̬
    bool               m_bMakeScript;     // ������scriptTag
    bool               m_bFirstKey;       // �Ѿ������һ���ؼ�֡
    bool               m_bRun;            // ִ��״̬

    CriticalSection    m_cs;
};

