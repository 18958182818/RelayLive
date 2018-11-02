/**
 * ���ĶԷ�ƽ̨
 */

#pragma once
#include "SipHeaders.h"

class CSipSubscribe
{
public:
    CSipSubscribe(eXosip_t* pSip);
    virtual ~CSipSubscribe(void);

    // ���涩����Ϣ
    void Subscribe(string strDevCode, string strAddrIP, string strAddrPort);

    // �¼�����
    void SubscribeAlarm(string strDevCode, string strAddrIP, string strAddrPort);

    // λ����Ϣ����
    void SubscribeMobilepostion(string strDevCode, string strAddrIP, string strAddrPort);

private:
    //���Ͷ�����Ϣ
    int SendSubscribe(CSipFromToHeader &from, 
                      CSipFromToHeader &to,
                      const int expires);

    //�����¼�������Ϣ
    int SendSubscribeAlarm(CSipFromToHeader &from, 
                      CSipFromToHeader &to,
                      const int expires);

    //����λ����Ϣ����
    int SendSubscribeMobilepostion(CSipFromToHeader &from, 
        CSipFromToHeader &to,
        const int expires);

private:
    eXosip_t* m_pExContext;
};

