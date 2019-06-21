#pragma once

#include "LiveClient.h"
#include "uv.h"
#include "ring_buff.h"

enum NalType;

namespace LiveClient
{
enum STREAM_TYPE;
class CLiveWorker;

/**
 * ��Ƶ��rtp/rtcp���մ���ģ��
 */
class CLiveReceiver
{
    friend class CLiveWorker;
public:
    CLiveReceiver(int nPort, CLiveWorker *worker);
    ~CLiveReceiver(void);

    /** ����UDP�˿ڼ��� */
    void StartListen();

    /** ���յ�rtp���ݴ��� */
    bool RtpRecv(char* pBuff, long nLen, struct sockaddr_in* addr_in);

    /** ���ճ�ʱ���� */
    void RtpOverTime();

    /** rtp���ݴ����߳� */
    void RtpParse();

    /**
     * ���PS��������
     * @param[in] buff PS����
     */
    void push_ps_stream(AV_BUFF buff);

    /**
     * ���PES��������
     * @param[in] buff PES������
     */
    void push_pes_stream(AV_BUFF buff);

    /**
     * ���ES��������
     * @param[in] buff ES������
     * @param[in] pts չ��ʱ����ֶ�
     * @param[in] dts ����ʱ����ֶ�
     */
    void push_es_stream(AV_BUFF buff, uint64_t  pts, uint64_t  dts);

    /**
     * ���h264��������
     * @param[in] buff H264֡����
     */
    void push_h264_stream(AV_BUFF buff);


    /** ����ʱ�ر�loop */
    void AsyncClose();

public:
    bool        m_bRtpRun;          // rtp �����Ƿ���ִ��
    bool        m_bTimeOverRun;     // ��ʱ�ж��Ƿ���ִ��
    bool        m_bRun;             // loop�Ƿ�����ִ��
    uv_loop_t   *m_uvLoop;          // udp���յ�loop

private:
    int         m_nLocalRTPPort;    // ����RTP���ն˿�
    int         m_nLocalRTCPPort;   // ����RTCP���ն˿�
    string      m_strRemoteIP;      // Զ�˷���IP
    int         m_nRemoteRTPPort;   // Զ��RTP���Ͷ˿�
    int         m_nRemoteRTCPPort;  // Զ��RTCP���Ͷ˿�

    uv_udp_t    m_uvRtpSocket;      // rtp����
    uv_timer_t  m_uvTimeOver;       // ���ճ�ʱ��ʱ��
    uv_async_t  m_uvAsync;          // �첽������� �ⲿ�߳���������m_uvLoop

    void*       m_pRtpParser;       // rtp���Ľ�����
    void*       m_pPsParser;        // PS֡������
    void*       m_pPesParser;       // PES��������
    void*       m_pEsParser;        // ES��������
    CLiveWorker* m_pWorker;         // �ص�����

    uint64_t    m_pts;              // ��ʾʱ���
    uint64_t    m_dts;              // ����ʱ���
    NalType     m_nalu_type;        // h264ƬԪ����
    ring_buff_t* m_pRingRtp;        // rtp����������loop�߳�д�룬�����̶߳�ȡ
};

}

