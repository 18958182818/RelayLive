#pragma once
/**
 * ����������Ϣ
 */
class CSipConfig
{
public:
    CSipConfig():bRegAuthor(false){};
    ~CSipConfig(){};
    /**
     * ���������ļ�
     */
    void Load();

public:
    /** Sip���������� */
    string   strDevCode;       //< �������
    string   strAddrIP;        //< ����IP
    string   strAddrPort;      //< Sip�����˿�
    bool     bRegAuthor;       //< �Ƿ�����Ȩ
};
