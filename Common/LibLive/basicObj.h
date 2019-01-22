#pragma once

#include "libLive.h"

enum NalType;
enum STREAM_TYPE;

/**
 * RTSP����ģ��ӿ�
 * ʹ��ǰ����������ȳ�ʼ��<ָUdpSocket�����ʼ������ʹ��>
 */
class CBasicObj : public IlibLive
{
public:
    CBasicObj();
    ~CBasicObj();


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
    
    /**
     * ���ô������ݻص��Ķ���
     * @param[in] pHandle
     */
    void SetCallback(IlibLiveCb* pHandle)
    {
        m_pCallBack = pHandle;
    }

protected:
    void*       m_pH264;            // H264������
    void*       m_pTs;              // TS�����
    void*       m_pFlv;             // FLV�����
    void*       m_pMp4;             // MP4�����
    IlibLiveCb* m_pCallBack;        // �ص�����
};

