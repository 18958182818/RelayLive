#pragma once
#include "avtypes.h"
#include "ring_buff.h"
#include "uv.h"
#include "Netstreammaker.h"

	enum NalType;
namespace LiveClient
{

    class DECODETOOL;
    class CODECTOOL;

class CReCode
{
public:
    CReCode(void* handle=NULL);
    ~CReCode(void);

    /**
     * ����ͨ��1����Ϣ
     */
    void SetChannel1(uint32_t nWidth, uint32_t nHeight, AV_CALLBACK cb);

    /**
     * H264�������½���롢����
     */
    int ReCode(AV_BUFF buff, NalType t);

private:
	/**
     * ����һ����Ƶ�ϲ�����
     */
    bool MakeVideo(char *data,int size,int bIsKeyFrame);

	/**/
	bool MakeKeyVideo();

private:
    void            *m_hUser;

    DECODETOOL      *m_decode;
    CODECTOOL       *m_codec;

	CNetStreamMaker*        m_pSPS;            // ����SPS���ݵĻ���
    CNetStreamMaker*        m_pPPS;            // ����PPS���ݵĻ���
    CNetStreamMaker*        m_pKeyFrame;       // ����ؼ�֡��sps��pps�п����ں���
	bool               m_bFirstKey;       // �Ѿ������һ���ؼ�֡
    bool               m_bRun;            // ִ��״̬
    bool               m_bGotSPS;
    bool               m_bGotPPS;
	uint32_t           m_timestamp;       // ʱ���
    uint32_t           m_tick_gap;        // ��֡��ļ��
};

};