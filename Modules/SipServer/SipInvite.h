#pragma once

namespace SipServer {

/**
 * �Ự����
 */
class CSipInvite
{
public:
    /**
     * ���ͻỰ����
     * @param strDevID[in] ��Ƶ�豸����
     * @param nRTPPort[in] ����rtp�Ķ˿�
     * @return call-id
     */
    int SendInvite(string strProName, uint32_t nID, string strCode, int nRTPPort);
    int SendRecordInvite(string strProName, uint32_t nID, string strCode, int nRTPPort, string beginTime, string endTime);

    /**
     * ���յ�200OK����
     */
    void OnInviteOK(eXosip_event_t *osipEvent);
    void OnInviteFailed(eXosip_event_t *osipEvent);

    /**
     * �����Ự,ָ���˿ڵĻỰ
     */
    bool StopSipCall(uint32_t nRtpPort);

    /**
     * ����ָ��liveserver�����лỰ
     */
    vector<uint32_t> StopSipCallAll(string strProName);

    /**
     * �յ��½��Ự
     */
    void OnCallNew(eXosip_event_t *osipEvent);

    /**
     * �յ��Է�����ر�
     */
    void OnCallClose(eXosip_event_t *osipEvent);

    /**
     * �յ��Ự����
     */
    void OnCallClear(eXosip_event_t *osipEvent);
};

};