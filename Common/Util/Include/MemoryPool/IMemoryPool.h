#pragma once

#include "Mutex.h"
#include "ExportDefine.h"

#if 0
#define Log_Debug Log::debug
#else
#define Log_Debug
#endif

/** �ڴ�ӳ���Ԫ */
struct memory_block  
{  
    size_t count;       //< ���block������ֻ����ʼblock��Ч
    size_t start;       //< �����ʼblock������ֻ�Խ���block��Ч
    void* pmem_chunk;   //< memory_chunk��ַ��ֻ����ʼblock��Ч��Ϊnull��ʾ��ʹ�ã���Ϊnull��ʾ����
};
  
/**
 * �ڴ�ؽӿ�
 */
class COMMON_API IMemoryPool
{
public:
    IMemoryPool(void);
    virtual ~IMemoryPool(void);

    /**
     * ��ʼ���ڴ��
     * @param nUnitCount �ڴ浥Ԫ����
     * @param nUnitSize  �ڴ浥Ԫ��С
     * @return �ɹ�true��ʧ��false
     */
    virtual bool init(int nUnitCount, int nUnitSize);

    /**
     * �����ڴ��
     */
    virtual void clear() = 0;

    /**
     * �����ڴ�
     */
    virtual void* mp_malloc(size_t nSize) = 0;

    /**
     * �ͷ��ڴ�
     */
    virtual void mp_free(void* p) = 0;

    virtual void mp_copy(void* p,void* drc,size_t nSize);

protected:

    /**
     * ��ȡmemory chunk pool �Ĵ�С
     */
    virtual size_t get_chunk_pool_size() = 0;

    /**
     * �ڴ�ӳ����е�����ת��Ϊ�ڴ���ʼ��ַ
     */
    void* index2addr(size_t index)  
    {
        char* p = (char*)m_pMemory;  
        void* ret = (void*)(p + index *m_nUnitSize);  

        Log_Debug("index2addr:%d--%d",index,ret);

        return ret;  
    }

    /**
     * �ڴ���ʼ��ַת��Ϊ�ڴ�ӳ����е�����
     */
    size_t addr2index(void* addr)  
    {
        char* start = (char*)m_pMemory;  
        char* p = (char*)addr;  
        size_t index = (p - start) / m_nUnitSize;  
        Log_Debug("addr2index:%d--%d",index,addr);
        return index;  
    }

protected:
    size_t          m_nUnitCount;           //< �ڴ�ص�Ԫ��
    size_t          m_nUnitSize;            //< һ���ڴ�ص�Ԫ�Ĵ�С
    memory_block*   m_pMemoryMapTable;      //< memory map table�ĵ�ַ
    void*           m_pChunkPool;           //< Memory chunk pool�ĵ�ַ
    void*           m_pMemory;              //< ʵ�ʿɷ�����ڴ�����ַ
    size_t          m_nUsedSize;            // ��¼�ڴ�����Ѿ�������û����ڴ�Ĵ�С
    size_t          m_nUsedCount;           // �Ѿ�����ĵ�Ԫ��
    bool            m_bInit;                //< �Ƿ��ʼ��
    CriticalSection m_cs;                   //< �ڴ���
};

