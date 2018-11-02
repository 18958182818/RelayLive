#pragma once

#include "ExportDefine.h"
#include "Mutex.h"
#include <vector>
#include <string>
using namespace std;

struct ProcessInfo
{
    DWORD   m_lPID;         //< ����ID
    int     m_nNum;         //< ���̱��
    string  m_strCMD;       //< ���͸��ӽ��̵�����
    time_t  m_tStart;       //< ��������ʱ��
};

/**
 * ���̹������������ӽ��̵���������������ʱ����
 */
class COMMON_API CProcessMgr
{
public:
    CProcessMgr(void);
    ~CProcessMgr(void);

    /**
     * ����һ���ӽ���
     * @param nNum ͬһ���������ϵĽ���������
     * @param strDevInfo �ý��̴�����豸����Ϣ
     * @return true�ɹ���falseʧ��
     */
    bool RunChild(int nNum, string strDevInfo);

    /**
     * ���������߳�
     */
    bool Protect();

    /**
     * �����߳�
     */
    bool ProtectRun();

private:
    /**
     * �����ӽ���
     */
    bool RunChild(ProcessInfo* pro);

    /**
     * �������̣���ͨ���ܵ�д������
     * @param nNum ͬһ���������ϵĽ���������
     * @param strDevInfo �ý��̴�����豸����Ϣ
     * @return true�ɹ���falseʧ��
     */
    bool CreateChildProcess(int nNum, string strDevInfo, DWORD& lPID);

    /**
     * �Ƴ�������Ϣ
     * @param nNum ���̱��
     */
    bool Remove(int nNum);

    /**
     * ����һ������
     * @param lPID ����ID
     */
    bool Find(DWORD lPID);

    /**
     * ����һ������
     * @param lPID ����ID
     */
    bool Kill(DWORD lPID);

    vector<ProcessInfo*>    m_vecProcess;       //< �ӽ�����Ϣ
    CriticalSection         m_cs;               //< m_vecProcess����
    string                  m_strPath;          //< ִ�г���·��
};