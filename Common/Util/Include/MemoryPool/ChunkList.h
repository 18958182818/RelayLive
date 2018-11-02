#pragma once
#include "IMemoryPool.h"

/** ���õ��ڴ��ṹ�� */
struct memory_chunk  
{
    memory_block* pfree_mem_addr;  
    memory_chunk* pre;  
    memory_chunk* next;  
};

/**
 * ����ṹ
 */
class CChunkList
{
public:
    CChunkList(void);
    ~CChunkList(void);

    /**
     * ��������ȡ����ͷ
     * @return ����ͷ�ڵ�
     */
    memory_chunk* pop_front();

    /**
     * ��������ɾ��ָ���ڵ�
     */
    void delete_chunk(memory_chunk* element);

    /**
     * ������ͷ�����µĽڵ�
     * @param element �½ڵ�
     */
    void push_front(memory_chunk* element);

public:
    memory_chunk*       m_pHead;
    size_t              m_nCount;       //< �ڵ����
};

