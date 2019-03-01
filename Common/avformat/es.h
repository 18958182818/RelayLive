/**
 * ����ES��
 * ���H264Ƭ(֡)
 */
#pragma once
#include "liveObj.h"
#include "h264.h"


/**
 * ES��������
 */
class CES
{
public:
    CES(CLiveObj* pObj);
    ~CES(void);

    /**
     * ����һ��PES��
     * @param[in] pBuf PES֡
     * @param[in] nLen PES֡����
     * @return 0�ɹ� -1ʧ��
     */
    int InputBuffer(char* pBuf, uint32_t nLen);

private:
    /**
     * ��pes����������h264���ݻ��棬����һ��nalu����ڶ��pes����
     * @param[in] pBuf ��������
     * @param[in] nLen ���ݳ��� 
     */
    void CatchData(char* pBuf, uint32_t nLen);

private:
    CLiveObj*   m_pObj;                  // �ص��������
    char*       m_pH264Buf;
    uint32_t    m_nH264BufLen;
    int         m_nH264DataLen;
};

