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
class CES : public IAnalyzer
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
    int InputBuffer(char* pBuf, long nLen);

private:
    CLiveObj*   m_pObj;                  // �ص��������
};

