#pragma once

#include "basicObj.h"
#include <string>


/**
 * RTSP����ģ��ӿ�
 * ʹ��ǰ����������ȳ�ʼ��<ָUdpSocket�����ʼ������ʹ��>
 */
class CRtspObj : public CBasicObj
{
public:
    CRtspObj(std::string rtsp);
    ~CRtspObj(void);

    /** ����UDP�˿ڼ��� */
    void StartListen();

    /**
     * ES֡�����ص�
     * @param[in] pBuff H264֡����
     * @param[in] nLen H264֡����
     * @param[in] nNalType Nalu������
     */
    void ESParseCb(char* pBuff, long nLen/*, uint8_t nNalType*/);

    

    bool        m_bRun;
private:
    string      m_strRtspAddr;
};

