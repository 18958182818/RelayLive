#pragma once
#include "ExportDefine.h"
#include <string>
#include <windows.h>

/**
 * �ʼ��۷����
 */
class COMMON_API CMailSlotSever
{
public:
    CMailSlotSever(std::string strName);
    ~CMailSlotSever();

    void SetCallback(void (*p)(std::string));

private:
    static DWORD WINAPI ServeMailslot(LPVOID lpParameter);

private:
    bool        m_bStop;    //< ֹͣ����
    std::string m_strName;  //< �ʼ�������

    void (*fMessCallBack)(std::string); 
};

/**
 * �ʼ��ۿͻ���
 */
class COMMON_API CMailSlotClient
{
public:
    CMailSlotClient(std::string strName);
    ~CMailSlotClient();

    bool SendMail(std::string strInfo); 

private:
    std::string m_strName;  //< �ʼ�������
    HANDLE      m_hMailslot;    //< �ļ��۾��
};