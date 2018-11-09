#pragma once
#include "live_obj.h"
#include "NetStreamMaker.h"

enum flv_tag_type
{
    callback_flv_header = 0,
    callback_script_tag,
    callback_video_spspps_tag,
    callback_key_video_tag,
    callback_video_tag
};


class flv_buffer : public CNetStreamMaker
{
public:
    void append_amf_string(const char *str );
    void append_amf_double(double d );
} ;

class CFlv : public IAnalyzer
{
public:
    CFlv(CLiveObj* pObj);
    ~CFlv(void);

    int InputBuffer(char* pBuf, long nLen);

    /**
     * ����flv�ļ�ͷ��Ϣ������
     * @param ppBuff ���flvͷ����
     * @param pLen ���flvͷ����
     */
    static bool MakeHeader(char** ppBuff, int* pLen);

    /**
     * ����һ������������Ϣ��������
     */
    bool MakeScriptTag();

    /**
     * ����һ����Ƶh264ͷƬ�β�����
     */
    bool MakeVideoH264HeaderTag();

    /**
     * ����һ����Ƶ�ϲ�����
     */
    bool MakeVideoH264Tag(char *data,int size,int bIsKeyFrame);

private:
    flv_buffer*        m_pSPS;            // ����SPS���ݵĻ���
    flv_buffer*        m_pPPS;            // ����PPS���ݵĻ���
    flv_buffer*        m_pData;           // ������ɵ�flv����

    uint32_t           m_timestamp;       // ʱ���
    uint32_t           m_tick_gap;        // ��֡��ļ��

    CLiveObj*          m_pObj;

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

