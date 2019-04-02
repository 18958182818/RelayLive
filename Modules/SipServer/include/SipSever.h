/**
 * Sip������
 */
#pragma once

class CSipSever
{
public:
    CSipSever(eXosip_t* pSip);
    ~CSipSever(void);

    /**
     * ����sip������
     */
    void StartSever();

private:
    /**
     * sip�����������߳�
     */
    void SeverThread();

    /**
     * ��ʱ������Ϣ
     */
    void SubscribeThread();

    /**
     * ������յ�ע���¼�
     */
    void OnRegister(eXosip_event_t *osipEvent);

    /**
     * ������յ�����Ϣ�¼�
     */
    void OnMessage(eXosip_event_t *osipEvent);

    /**
     * ������յ���200OK�¼�
     */
    void OnMessageOK(eXosip_event_t *osipEvent);

    /**
     * ������յ���Invite 200OK�¼�
     */
    void OnInviteOK(eXosip_event_t *osipEvent);

    /**
     * �������е�
     */
    void OnCallNew(eXosip_event_t *osipEvent);

    /**
     * 
     */
    void OnCallClose(eXosip_event_t *osipEvent);

    void OnCallClear(eXosip_event_t *osipEvent);

private:
    eXosip_t*   m_pExContext;
    time_t      m_nSubTime; //����ʱ��
    bool        m_bSubStat; //�Ƿ����豸״̬
    bool        m_bSubPos;  //�Ƿ����豸λ��
    bool        m_bQueryDir;//�Ƿ���Ҫ�����豸
};

