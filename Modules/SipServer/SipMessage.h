#pragma once
#include "SipHeaders.h"
#include "SipMsgParser.h"

struct sipMessageInfo
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
    CSipFromToHeader to;
    // ��Ϣ��
    string content;
    // ��Ϣ����
    string strCmdType;
    // ��Ϣ�����
    map<string,string> mapBody;
    msgPublic* pMsgBody;
};

/**
 * ������յ���Ϣ��������Ӧ�𡣷���message����(��ѯĿ¼����̨����)
 */
class CSipMessage
{
public:
    CSipMessage(eXosip_t* pSip);
    ~CSipMessage(void);

    /**
     * ����Message�¼�
     */
    void OnMessage(eXosip_event_t *osipEvent);

    /**
     * Ŀ¼��ѯ
     * @param strDevCode[in] ƽ̨����
     * @param strAddrIP[in] ƽ̨IP
     * @param strAddrPort[in] ƽ̨�˿�
     */
    void QueryDirtionary(string strDevCode, string strAddrIP, string strAddrPort);

    /**
     * �豸״̬��ѯ
     * @param strDevCode[in] ƽ̨����
     * @param strAddrIP[in] ƽ̨IP
     * @param strAddrPort[in] ƽ̨�˿�
     * @param devID[in] �豸id
     */
    void QueryDeviceStatus(string strDevCode, string strAddrIP, string strAddrPort, string devID);

    /**
     * �豸��Ϣ��ѯ����
     * @param strDevCode[in] ƽ̨����
     * @param strAddrIP[in] ƽ̨IP
     * @param strAddrPort[in] ƽ̨�˿�
     * @param devID[in] �豸id
     */
    void QueryDeviceInfo(string strDevCode, string strAddrIP, string strAddrPort, string devID);

    /**
     * �ļ�Ŀ¼��������
     * @param strDevCode[in] ƽ̨����
     * @param strAddrIP[in] ƽ̨IP
     * @param strAddrPort[in] ƽ̨�˿�
     * @param devID[in] �豸id
     */
    void QueryRecordInfo(string strDevCode, string strAddrIP, string strAddrPort, string devID, string strStartTime, string strEndTime);

    /**
     * �ƶ��豸λ�ò�ѯ
     * @param strDevCode[in] ƽ̨����
     * @param strAddrIP[in] ƽ̨IP
     * @param strAddrPort[in] ƽ̨�˿�
     * @param devID[in] �豸id
     */
    void QueryMobilePosition(string strDevCode, string strAddrIP, string strAddrPort, string devID);

    /**
     * ��̨����
     * @param strPlatformCode[in] ƽ̨����
     * @param strAddrIP[in]   ƽ̨IP
     * @param strAddrPort[in] ƽ̨�˿�
     * @param strDevCode[in] �豸����
     * @param nInOut[in]     ��ͷ�Ŵ���С 0:ֹͣ 1:��С 2:�Ŵ�
     * @param nUpDown[in]    ��ͷ�������� 0:ֹͣ 1:���� 2:����
     * @param nLeftRight[in] ��ͷ�������� 0:ֹͣ 1:���� 2:����
     * @param cMoveSpeed[in] ��ͷ�����ٶ�
     * @param cMoveSpeed[in] ��ͷ�ƶ��ٶ�
     */
    void DeviceControl(string strAddrIP, string strAddrPort, string strDevCode,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0, 
        uchar cInOutSpeed = 0X1, uchar cMoveSpeed = 0XFF);
private:
    /**
     * ������Ϣ��Ϣ
     */
    void parserMessageInfo(osip_message_t*request, int iReqId, sipMessageInfo &msgInfo);

    /**
     * ��ӡ���յ�����Ӧ����
     */
    void printMeaassgePkt(sipMessageInfo& info);

    /**
     * ����Ӧ��
     */
    void sendMessageAnswer(sipMessageInfo& info);

private:
    eXosip_t*      m_pExContext;
    CSipMsgParser  m_msgParser;   //< ��Ϣ���������
};

