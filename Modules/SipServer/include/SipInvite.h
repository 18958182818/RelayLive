#pragma once
#include "SipMsgParser.h"

/**
 * �Ự����
 */
class CSipInvite
{
public:
    CSipInvite(eXosip_t* pSip);
    ~CSipInvite(void);

    /**
     * ���ͻỰ����
     * @param pPlatform[in] �¼�ƽ̨��Ϣ
     * @param pDevInfo[in] ��Ƶ�豸��Ϣ
     * @param nRTPPort[in] ����rtp�Ķ˿�
     * @return call-id
     */
    int SendInvite(PlatFormInfo* pPlatform, DevInfo* pDevInfo, int nRTPPort);
    int SendRecordInvite(PlatFormInfo* pPlatform, DevInfo* pDevInfo, int nRTPPort, string beginTime, string endTime);

    /**
     * ���յ�200OK����
     */
    void OnInviteOK(eXosip_event_t *osipEvent);
    void OnInviteFailed(eXosip_event_t *osipEvent);

    /**
     * �����Ự
     */
    void SendBye(int cid, int did);

    /**
     * 
     */
    void OnCallNew(eXosip_event_t *osipEvent);

    /**
     * 
     */
    void OnCallClose(eXosip_event_t *osipEvent);

    /**
     * 
     */
    void OnCallClear(eXosip_event_t *osipEvent);
private:
    eXosip_t*      m_pExContext;
};

