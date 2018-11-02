#pragma once
#include "imemorypool.h"
#include "ExportDefine.h"

/**
 * ����ṹ���ڴ��
 * ����ṹ���ڴ��ʵ����ָ��memory chunk setʵ��Ϊ˫������ṹ�����ַ�������ȱ�����£�
 * �ŵ㣺�ͷ��ڴ�ܿ죬O(1)���Ӷȡ�	
 * ȱ�㣺�����ڴ������O(n)���Ӷȡ�
 */
class COMMON_API CMemoryPoolList : public IMemoryPool
{
public:
    CMemoryPoolList(void);
    ~CMemoryPoolList(void);

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

protected:
    
    /**
     * ��ȡmemory chunk pool �Ĵ�С
     */
    virtual size_t get_chunk_pool_size() override;

private:
    void*           m_pEmptyNodeList;   //< chunk pool ��δʹ�õ�chunk�ڵ���������Ա�ʹ��
    void*           m_pFreeChunkList;   //< ָ������ڴ���chunk����
};

