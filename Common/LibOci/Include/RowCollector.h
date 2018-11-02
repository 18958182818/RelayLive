#pragma once
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include "OciDefine.h"

/**
 * ���ݼ�¼�����������������ﵽһ����������һ��ʱ��ʱ��ͨ���ص�������������
 */
class LIBOCI_API RowCollector
{
public:
    /**
     * ���캯��
     * @param num_rows �������������ݽ����ύ
     * @param interval �������ʱ�����һ���ύ
     */
	RowCollector(void);
	RowCollector(std::size_t num_rows, std::size_t interval);
	~RowCollector(void);

    /**
     * ���ûص�����
     */
    void set_info(std::size_t num_rows, std::size_t interval);

    /**
     * ����һ������
     */
	void add_row(vector<string> row);

    /**
     * ��ӻص�����
     */
	void add_insertion_handler(function<void(std::vector<vector<string>>)> handler);
	
private:

    /**
     * ��ʱɨ�������̣߳����������ݲ��㵫��ʱҲҪ����
     * @call move_rows()
     * @call insert()
     */
	void collect();
    /**
     * �ӽ�������ָ�������������ƶ�����������
     */
	int move_rows();

    /**
     * ���ûص�������������
     */
	void insert();

private:
	std::queue<vector<string>>   m_queReciveRows;               //���յ�������
	std::vector<vector<string>>  m_vecInsertRows;               //����������
	std::size_t     m_nIntervalSeconds;            //���������λΪ��
	std::size_t     m_nMaxRows;                    //������������

	std::thread     m_thread;
    std::mutex      m_mtx;           //add_row()��collect()�����߳�֮�以��
    bool            m_bRun;

	std::chrono::system_clock::time_point   m_tpInsertTime;     //< ��һ�λص�����ʱ��
	vector<function<void(std::vector<vector<string>>)>>  m_vecCallbackFun;   //< �ص�����
};
