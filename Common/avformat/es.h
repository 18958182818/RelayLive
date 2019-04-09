/**
 * ����ES��
 * ���H264Ƭ(֡)
 */
#pragma once
#include "liveObj.h"
#include "h264.h"

typedef void (*ES_CALLBACK)(char*, long, void*);

/**
 * ES��������
 */
class CES
{
public:
    CES(void* handle, ES_CALLBACK cb);
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
    void*             m_hUser;                  // �ص��������
    ES_CALLBACK       m_fCB;

    char*       m_pH264Buf;
    uint32_t    m_nH264BufLen;
    int         m_nH264DataLen;
};

