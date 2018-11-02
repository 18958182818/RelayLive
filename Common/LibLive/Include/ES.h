/**
 * ����ES��
 * ���H264Ƭ(֡)
 */
#pragma once
#include "LiveInstance.h"
#include "H264.h"


/**
 * ES��������
 */
class CES : public IAnalyzer
{
public:
    CES(CLiveInstance* pObj);
    ~CES(void);

    /**
     * ����һ��PES��
     * @param[in] pBuf PES֡
     * @param[in] nLen PES֡����
     * @return 0�ɹ� -1ʧ��
     */
    int InputBuffer(char* pBuf, long nLen);

private:
    CLiveInstance*   m_pObj;                  // �ص��������
};

