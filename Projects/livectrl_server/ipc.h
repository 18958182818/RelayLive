#pragma once
#include "SipServer.h"
#include <string>

namespace IPC {
    /**
     * ��ʼ��IPC
     */
    bool Init();

    /**
     * ����IPC
     */
    void Cleanup();

    /**
     * ��ȡ�ͻ�����Ϣ��json
     */
    std::string GetClientsJson();

    /**
     * ��ȡ�豸�б��json
     */
    std::string GetDevsJson();

    /**
     * �豸��Ϣ����
     */
    void AddDev(SipServer::DevInfo *dev);

    /**
     * ˢ���豸��ѯ
     */
    void DevsFresh();

    /**
     * �豸����
     */
    void DevControl(string strDev, int nInOut = 0, int nUpDown = 0, int nLeftRight = 0);
};