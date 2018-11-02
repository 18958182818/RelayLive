#pragma once
#include "ocilib.h"
#include "OciDefine.h"

/** ����������ʱ�䣬������ӳ������ʱ��һֱ���ڿ���״̬���ͽ����Ƴ� */
#define ORA_CONN_MAX_IDLE_TIME 60 * 10

/** ����״̬ */
enum cn_state
{
	cs_idle = 0,     // ����
	cs_busy = 1,     // ����
	cs_inactive = 2  // ʧЧ
};

/** ���ӽṹ�� */
struct DBConnection
{
	OCI_Connection *cn;      //oci����ʵ��
	cn_state cstate;         //����״̬
	time_t idle_begin_time;  //�ۼƿ���ʱ��
};

/**
 * һ���Ự��ַ�����ӳ�
 */
class LIBOCI_API ConnPool
{
public:
    /**
     * ���캯��
     * @param db ���ݿ��ַ
     * @param user ��¼�û���
     * @param psw ��¼����
     * @param min ��С������
     * @param max ���������
     * @param inc ÿ����ഴ����������
     */
	ConnPool(std::string db, std::string user, std::string psw);
	ConnPool(std::string db, std::string user, std::string psw, int min, int max, int inc);
	~ConnPool(void);

	void run();

    /**
     * ����ID��ȡ����ʵ������
     * @param id �������ӵ�id
     * @return ����ʵ��
     */
	OCI_Connection *at(int id);

    /**
     * �ӳ����л�ȡ�и��������ӵ�id
     * @param timeout ִ�еĳ�ʱʱ�䣬��λ���롣Ĭ�ϳ�ʱʱ��Ϊ2��
     * @return �������ӵ�id�����û��ȡ����Ϊ-1
     * @remark �ɹ���ȡ�󣬳�����������ӵ�״̬�����idle���busy
     */
	int getConnection(int timeout=2000);

    /**
     * ��һ�����ӻ��ص�������
     * @param id ��Ҫ�黹�����ӵ�id
     * @remark �黹�󣬳�����������ӵ�״̬�����busy���idle
     */
	void releaseConnection(int id);
	
    /**
     * ��ȡ���ڹ���״̬�����ӵĸ���
     */
	int getBusyCount();

    /**
     * ��ȡ��ǰ�ܵ����Ӹ���
     */
	int getOpenedCount(); 
	
private:
    /**
     * ��ʼ��ʱ��������
     */
	void createPool();
    /**
     * �ͷ���������
     */
	void freePool();
    /**
     * ��������
     * @param num ���Ӹ���
     * @return �ɹ����е����Ӹ���
     */
	int createConnection(int num);

    /**
     * �Ƴ�ָ�������ӣ������ǿ���״̬
     * @param id ָ������id
     * @return �ɹ�true��ʧ��false
     */
	bool removeConnection(int id);  

    /**
     * ��������Ƿ���Ч
     */
	bool checkConnection(OCI_Connection *cn);

    /**
     * ���ؿ�������Ч������ID
     */
	int findIdle();

    /**
     * ��ʱ�����̳߳�ά����ɾ��ʧЧ�ͳ�ʱ����
     */
	void maintain();

    /**
     * �������ʧЧ�ͳ�ʱ������
     * @param invalid_conns[out] ���ʧЧ�ͳ�ʱ���ӵ�ID
     * @param max_idle_time[in] ��ʱʱ�䣬��λ�Ǻ���
     * @return ��Ч��ʱ�����ӵĸ���
     */
	int find_invalid_connections(std::vector<int>& invalid_conns, int max_idle_time);

private:
	std::string                 m_strDatabase;              //< ���ݿ��ַ
	std::string                 m_strUserName;              //< ��¼�û���
	std::string                 m_strPassword;              //< ��¼����
	int                         m_nMinConnectNum;           //< ��С������
	int                         m_nMaxConnectNum;           //< ���������
	int                         m_nIncreaseConnectNum;      //< ÿ�δ�����������

	std::map<int, DBConnection> m_mapConnects;              //< ���ݿ����ӳ�
	std::mutex                  m_mtxConnects;              //< ���ݿ����ӳص���
	int                         m_nNextConnectID;           //< �½����������õ�id
	int                         m_nMaxIdleTime;             //< ����Ŀ���ʱ�䣬��λΪ�룬�������Ƴ�
	
	int                         m_nConnectNum;              //< ���е��������������ܰ����ǻ�Ծ������
	int                         m_nBusyConnectNum;          //< ���е�ǰ���ڹ�����������

	std::thread                 m_threadMaintain;           //< ���ӳص�ά���߳�
    bool                        m_bRunning;                 //< �߳�������
};

typedef std::shared_ptr<ConnPool> conn_pool_ptr;

