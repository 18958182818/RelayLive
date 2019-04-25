#pragma once

#include "LiveClient.h"
#include "uv.h"

enum NalType;
enum FLV_FRAG_TYPE;
enum MP4_FRAG_TYPE;

namespace LiveClient
{
enum STREAM_TYPE;
class CLiveWorker;

/**
 * ��Ƶ��rtp/rtcp���մ���ģ��
 */
class CLiveObj
{
public:
    CLiveObj(int nPort, CLiveWorker *worker);
    ~CLiveObj(void);

    /** ����UDP�˿ڼ��� */
    void StartListen(string strRemoteIP, int nRemotePort);

    /** ���յ�rtp���ݴ��� */
    void RtpRecv(char* pBuff, long nLen, struct sockaddr_in* addr_in);

    /** ���ճ�ʱ���� */
    void RtpOverTime();

    /**
     * RTP����ص���rtp����ɵ�֡������ps��Ҳ������h264
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

    /** H264��sps�����ص� */
    void H264SpsCb(uint32_t nWidth, uint32_t nHeight, double fFps);

    /** FLV�ϳɻص� */
    void FlvCb(FLV_FRAG_TYPE eType, char* pBuff, int nBuffSize);

    /** MP4�ϳɻص� */
    void Mp4Cb(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize);

    /** TS�ϳɻص� */
    void TsCb(char* pBuff, int nBuffSize);

    /** H264�ϳɻص� */
    void H264Cb(char* pBuff, int nBuffSize);

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
    void*       m_pH264;            // H264������
    void*       m_pTs;              // TS�����
    void*       m_pFlv;             // FLV�����
    void*       m_pMp4;             // MP4�����
    CLiveWorker* m_pWorker;        // �ص�����

    uint64_t    m_pts;              // ��¼PES�е�pts
    uint64_t    m_dts;              // ��¼PES�е�dts
    NalType     m_nalu_type;        // h264ƬԪ����
};

}

