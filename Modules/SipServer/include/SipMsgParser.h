/**
 * ����������յ��ı����У�����MANSDP+XML��ʽ�ı���body����
 */
#pragma once
#include "pugixml.hpp"
#include "PublicDefine.h"

struct msgPublic
{
    string strCmdType;  //< ��������
    string strSN;       //< �������к� 
    string strDeviceID; //< Դ�豸/ϵͳ����
};

/** ֪ͨ���� */
struct msgNotify : public msgPublic
{
};

/** Ӧ������ */
struct msgResponse : public msgPublic
{
};

/** ״̬��Ϣ���� */
struct msgKeepAlive : public msgNotify
{
    string         strStatus;   //< �Ƿ���������
    vector<string> vecDeviceID; //< �����豸�б�
};

/** �豸Ŀ¼��ѯ��ϢӦ�� */
struct msgDevDirQuery : public msgResponse
{
    string            strSumNum;   //< ��ѯ�������
    vector<DevInfo*> vecDevInfo;  //< �豸�б�
};

/**
 * ��������
 */
class CSipMsgParser
{
public:
    CSipMsgParser(void);
    ~CSipMsgParser(void);

    /**
     * ��������
     * @param ppMsg[out] ���������Ľṹ��
     * @param szBody[in] ���뱨��������
     * @return �������� ��������+CmdType
     */
    string ParseMsgBody(msgPublic** ppMsg, const char* szBody);

private:
    /**
     * ֪ͨ����Ϣ����
     * @param ppMsg[out] ���������Ľṹ��
     * @param root[in]   ����xml���ڵ�
     * @return ������������CmdType
     */
    string ParseNotify(msgPublic** ppMsg, pugi::xml_node& root);

    /**
     * Ӧ������Ϣ����
     * @param ppMsg[out] ���������Ľṹ��
     * @param root[in]   ����xml���ڵ�
     * @return ������������CmdType
     */
    string ParseResponse(msgPublic** ppMsg, pugi::xml_node& root);

    /**
     * ����״̬��Ϣ����
     */
    msgPublic* ParseKeepAlive(pugi::xml_node& root);

    /**
     * �����豸Ŀ¼��Ϣ��ѯ����
     */
    msgPublic* ParseCatalog(pugi::xml_node& root);

    /**
     * Ŀ¼��Ϣ���ĺ��յ������ͱ���
     */
    msgPublic* ParseNotifyCatalog(pugi::xml_node& root);

    /**
     * λ����Ϣ���ĺ��յ������ͱ���
     */
    msgPublic* ParseMobilePosition(pugi::xml_node& root);
};

