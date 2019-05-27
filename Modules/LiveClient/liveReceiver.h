#pragma once

#include "LiveClient.h"
#include "uv.h"

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
    void RtpRecv(char* pBuff, long nLen, struct sockaddr_in* addr_in);

    /** ���ճ�ʱ���� */
    void RtpOverTime();

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

    /** H264��sps�����ص� */
    void set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps);

    /** FLV�ϳɻص� */
    void FlvCb(AV_BUFF buff);

    /** MP4�ϳɻص� */
    void Mp4Cb(AV_BUFF buff);

    /** TS�ϳɻص� */
    void TsCb(AV_BUFF buff);

    /** H264�ϳɻص� */
    void H264Cb(AV_BUFF buff);

public:
    bool        m_bRtpRun;
    bool        m_bTimeOverRun;

private:
    int         m_nLocalRTPPort;    // ����RTP���ն˿�
    int         m_nLocalRTCPPort;   // ����RTCP���ն˿�
    string      m_strRemoteIP;      // Զ�˷���IP
    int         m_nRemoteRTPPort;   // Զ��RTP���Ͷ˿�
    int         m_nRemoteRTCPPort;  // Զ��RTCP���Ͷ˿�

    uv_udp_t    m_uvRtpSocket;      // rtp����
    uv_timer_t  m_uvTimeOver;       // ���ճ�ʱ��ʱ��

    void*       m_pRtpParser;       // rtp���Ľ�����
    void*       m_pPsParser;        // PS֡������
    void*       m_pPesParser;       // PES��������
    void*       m_pEsParser;        // ES��������
    void*       m_pH264;            // H264������
    void*       m_pTs;              // TS�����
    void*       m_pFlv;             // FLV�����
    void*       m_pMp4;             // MP4�����
    void*       m_pReCode;          // �ر���
    CLiveWorker* m_pWorker;        // �ص�����

    uint64_t    m_pts;              // ��¼PES�е�pts
    uint64_t    m_dts;              // ��¼PES�е�dts
    NalType     m_nalu_type;        // h264ƬԪ����
};

}

