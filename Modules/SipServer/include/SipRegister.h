#pragma once
#include "SipHeaders.h"

struct SipContextInfo
{
    //Sip�㷵�ص�����ı�־ ��Ӧʱ���ؼ���
    int sipRequestId;
    //ά��һ��ע��
    string callId;
    //��Ϣ�����Ĺ��ܷ������ַ���
    string method;
    //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
    CSipFromToHeader from;
    //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
    CSipFromToHeader proxy;
    //��ַ����@������IP��ַ:���Ӷ˿ڣ�����sip:1111@127.0.0.1:5060
    CContractHeader contact;
    //��Ϣ����,һ��ΪDDCP��Ϣ��XML�ĵ�,���߾���Э��֡Ҫ��������ַ����ı�
    string content;
    //��Ӧ״̬��Ϣ
    string status;
    //��ʱ,ʱ�䵥λΪ��
    int expires;
};

struct SipAuthInfo
{
    //ƽ̨������
    string digestRealm;
    //ƽ̨�ṩ�������
    string nonce;
    //�û���
    string userName;
    //����
    string response;
    //��sip:ƽ̨��ַ��,����Ҫuac��ֵ
    string uri;
    //�����㷨MD5
    string algorithm;
};

struct sipRegisterInfo
{
    SipContextInfo baseInfo;
    SipAuthInfo authInfo;
    bool isAuthNull;             //< true:�޼�Ȩ��Ϣ��false:�м�Ȩ��Ϣ
};

/**
 * ������յ�ע������
 */
class CSipRegister
{
public:
    CSipRegister(eXosip_t* pSip);
    ~CSipRegister(void);

    /**
     * �����Ƿ�����Ȩ
     * @param bAuth true:��Ҫ��Ȩ��false:����Ҫ��Ȩ
     */
    void SetAuthorization(bool bAuth);

    /**
     * ����ע���¼�
     */
    void OnRegister(eXosip_event_t *osipEvent);

private:
    /**
     * ����ע����Ϣ
     * @param 
     */
    void parserRegisterInfo(osip_message_t*request, int iReqId, sipRegisterInfo &regInfo);

    //��ӡ���յ�����Ӧ����
    void printRegisterPkt(sipRegisterInfo& info);

    /**
     * ����Ӧ��
     */
    void sendRegisterAnswer(sipRegisterInfo& info);

private:
    eXosip_t* m_pExContext;
    string    m_strNonce;       //< UAS��ֵ����֤�����
    string    m_strAlgorithm;   //< UASĬ�ϼ����㷨
    bool      m_bAuthorization; //< �Ƿ���Ҫ��Ȩ
};

