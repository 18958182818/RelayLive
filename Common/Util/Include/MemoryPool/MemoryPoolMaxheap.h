/**
 * �󶥶ѽṹ�ڴ��
 * �󶥶ѽṹ���ڴ��ʵ����ָ��memory chunk setʵ��Ϊ�󶥶ѽṹ�����ַ�������ȱ�����£�
 * �ŵ㣺�����˷����ڴ��ʱ�临�Ӷȣ�O(log(n))��	
 * ȱ�㣺�������ͷ��ڴ��ʱ�临�Ӷȣ�O(log(n))��
 */

#pragma once
#include "imemorypool.h"
#include "ExportDefine.h"

struct max_heap;

class COMMON_API CMemoryPoolMaxheap : public IMemoryPool
{
public:
    CMemoryPoolMaxheap(void);
    ~CMemoryPoolMaxheap(void);

    /**
     * ��ʼ���ڴ��
     * @param nUnitCount �ڴ浥Ԫ����
     * @param nUnitSize  �ڴ浥Ԫ��С
     * @return �ɹ�true��ʧ��false
     */
    virtual bool init(int nUnitCount, int nUnitSize) override;

    /**
     * �����ڴ��
     */
    virtual void clear() override;

    /**
     * �����ڴ�
     */
    virtual void* mp_malloc(size_t nSize) override;

    /**
     * �ͷ��ڴ�
     */
    virtual void mp_free(void* p) override;

    void showinfo();

protected:
    
    /**
     * ��ȡmemory chunk pool �Ĵ�С
     */
    virtual size_t get_chunk_pool_size() override;

private:
    void*           m_pMaxHeap;
};

