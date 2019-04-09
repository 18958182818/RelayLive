#include "stdafx.h"
#include "es.h"


CES::CES(CLiveObj* pObj)
    : m_pObj(pObj)
    , m_pH264Buf(NULL)
    , m_nH264DataLen(0)
{
}


CES::~CES(void)
{
}

int CES::InputBuffer(char* pBuf, uint32_t nLen)
{
    // ES��û��esͷ����h264Ƭ���
    //nal_unit_header_t* nalUnit = nullptr;
    //uint8_t nNalType = 0;
    uint32_t pos = 0;
    uint32_t begin_pos = 0;
    char* begin_buff = pBuf;
    while (pos < nLen)
    {
        char* pPos = pBuf + pos;
        if (pPos[0] == 0 && pPos[1] == 0 && pPos[2] == 1)
        {
            //if(pos > 0) Log::debug("nal3 begin width 001 pos:%d",pos);
            // ��h264���Ŀ�ͷ
            if (pos > begin_pos)
            {
                // �ص�����H264֡
                //if (m_pObj != nullptr)
                //{
                //    m_pObj->ESParseCb(begin_buff, pos-begin_pos/*, nNalType*/);
                //}
                CatchData(begin_buff, pos-begin_pos);
            }
            //nalUnit = (nal_unit_header_t*)(pPos+3);
            //nNalType = nalUnit->nal_type;
            begin_pos = pos;
            begin_buff = (char*)pPos;
            pos += 3;
        }
        else if (pPos[0] == 0 && pPos[1] == 0 && pPos[2] == 0 && pPos[3] == 1)
        {
			//if(pos > 0) Log::debug("nal4 begin width 0001 pos:%d",pos);
            // ��h264���Ŀ�ͷ
            if (pos > begin_pos)
            {
                // �ص�����H264֡
                //if (m_pObj != nullptr)
                //{
                //    m_pObj->ESParseCb(begin_buff, pos-begin_pos/*, nNalType*/);
                //}
                CatchData(begin_buff, pos-begin_pos);
            }
            //nalUnit = (nal_unit_header_t*)(pPos+4);
            //nNalType = nalUnit->nal_type;
            begin_pos = pos;
            begin_buff = (char*)pPos;
            pos += 4;
        }
        else
        {
            // ����h264��ͷ
            pos++;
        }
    }
    // ���һ֡
    if (nLen > begin_pos)
    {
        // �ص�����H264֡
        //if (m_pObj != nullptr)
        //{
        //    m_pObj->ESParseCb(begin_buff, nLen-begin_pos/*, nNalType*/);
        //}
        CatchData(begin_buff, pos-begin_pos);
    }
    return 0;
}


void CES::CatchData(char* pBuf, uint32_t nLen) 
{
    if(m_pH264Buf == NULL)
    {
        m_pH264Buf = (char*)malloc(nLen);
        m_nH264BufLen = nLen;
    }

    bool begin = false;
    if ((pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1)
        || (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 0 && pBuf[3] == 1))
    {
        begin = true;
    }

    if (begin)
    {
        if(m_nH264DataLen > 0) {
            if (m_pObj != nullptr)
            {
                m_pObj->ESParseCb(m_pH264Buf, m_nH264DataLen);
            }
            m_nH264DataLen = 0;
        }
    }
    else
    {
        //������nalu��ʼ������Ϊ���ݵĿ�ͷ
        if(m_nH264DataLen == 0) {
            Log::error("this is not nalu head");
            return;
        }
    }

    int nNewLen = m_nH264DataLen + nLen;
    if(nNewLen > m_nH264BufLen) {
        m_pH264Buf = (char*)realloc(m_pH264Buf, nNewLen);
        m_nH264BufLen = nNewLen;
    }

    memcpy(m_pH264Buf+m_nH264DataLen, pBuf, nLen);
    m_nH264DataLen += nLen;
}