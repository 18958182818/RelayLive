#pragma once

#include "basicObj.h"
#include "uv.h"

enum NalType;
enum STREAM_TYPE;

/**
 * RTSP����ģ��ӿ�
 * ʹ��ǰ����������ȳ�ʼ��<ָUdpSocket�����ʼ������ʹ��>
 */
class CLiveObj : public CBasicObj
{
public:
    CLiveObj(liblive_option opt);
    ~CLiveObj(void);

    /** ����UDP�˿ڼ��� */
    void StartListen();

    /** ���յ�rtp���ݴ��� */
    void RtpRecv(char* pBuff, long nLen);

    /** ���ճ�ʱ���� */
    void RtpOverTime();

    /**
     * RTP����ص�
     * @param[in] pBuff PS֡����
     * @param[in] nLen PS֡����
     */
    void RTPParseCb(char* pBuff, long nLen);

    /**
     * PS֡�����ص�
     * @param[in] pBuff PES������
     * @param[in] nLen PES������
     */
    void PSParseCb(char* pBuff, long nLen);

    /**
     * PES֡�����ص�
     * @param[in] pBuff ES������
     * @param[in] nLen ES������
     * @param[in] pts չ��ʱ����ֶ�
     * @param[in] dts ����ʱ����ֶ�
     */
    void PESParseCb(char* pBuff, long nLen, uint64_t pts, uint64_t dts);

    /**
     * ES֡�����ص�
     * @param[in] pBuff H264֡����
     * @param[in] nLen H264֡����
     * @param[in] nNalType Nalu������
     */
    void ESParseCb(char* pBuff, long nLen/*, uint8_t nNalType*/);



    /** ����ʱ�ر�loop */
    void AsyncClose();

    bool        m_bRun;
    uv_loop_t   *m_uvLoop;
private:
    int         m_nLocalRTPPort;    // ����RTP�˿�
    int         m_nLocalRTCPPort;   // ����RTCP�˿�
    string      m_strRemoteIP;      // Զ��IP
    int         m_nRemoteRTPPort;   // Զ��RTP�˿�
    int         m_nRemoteRTCPPort;  // Զ��RTCP�˿�

    uv_udp_t    m_uvRtpSocket;      // rtp����
    uv_timer_t  m_uvTimeOver;       // ���ճ�ʱ��ʱ��
    uv_async_t  m_uvAsync;          // �첽�������

    void*       m_pRtpParser;       // rtp���Ľ�����
    void*       m_pPsParser;        // PS֡������
    void*       m_pPesParser;       // PES��������
    void*       m_pEsParser;        // ES��������


    uint64_t    m_pts;              // ��¼PES�е�pts
    uint64_t    m_dts;              // ��¼PES�е�dts
    NalType     m_nalu_type;        // h264ƬԪ����
};

