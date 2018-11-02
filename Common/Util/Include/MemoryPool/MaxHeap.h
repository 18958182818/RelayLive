#pragma once
#include "IMemoryPool.h"

/** ���õ��ڴ��ṹ�� */
struct memory_chunk  
{  
    memory_block*   pfree_mem_addr;     //< chunkָ���memory map table�е�δʹ�õ���ʼblock 
    size_t          chunk_size;         //< ������δʹ�õ�block�ĸ���
};

class CMaxHeap
{
public:
    CMaxHeap(void);
    ~CMaxHeap(void);

    /**
     * ��ʼ���󶥶�
     * @param max_heap_size �ѳ�Ա�����
     * @param heap_arr ��Ŵ󶥶����ݵ��ڴ��ַ
     */
    void init_max_heap(size_t max_heap_size, memory_chunk* heap_arr);

    /** �󶥶��Ƿ�Ϊ�� */
    bool is_heap_empty();

    /** �󶥶��Ƿ����� */
    bool is_heap_full();

    /**
     * ��ָ��λ�ÿ�ʼ�����Ͻ�������
     * @param start ָ�����λ��
     * @return ָ������������ڵ�λ��
     * @note �󶥶ѱ���������ģ�ֻ������һ��ֵ����������������Ҫ�����ʼ��������һ��
     */
    memory_chunk* filter_up(size_t start);

    /**
     * ��ָ��λ�ÿ�ʼ�����½�������
     * @param start ָ�����λ��
     * @return ָ������������ڵ�λ��
     * @note �󶥶ѱ���������ģ�ֻ������һ��ֵ��С��ɾ���������Ҫ�����ʼ��������һ��
     */
    memory_chunk* filter_down(size_t start);

    /** ����һ����Ա */
    memory_chunk* insert_heap(memory_chunk& chunk);

    /**
     * ��ȡ����Ա�������ϵĳ�Ա
     * @param chunk[out] �������Ա
     */
    bool get_max(memory_chunk*& chunk);

    /** �Ƴ����ϵ���(�����) */
    bool remove_max();

    /** �Ƴ�ָ���ֻ��һ��������õ��������ͷ��ڴ������chunk����block��ǰ����block�飬���ߺϲ���һ��block�顣��ʵ����ԭ��������chunk���һ�� */
    void remove_element(memory_chunk* chunk);

    /**
     * ָ�����chunksize���Ӵ�С
     * @param chunk Ҫ�Ķ�����
     * @param increase_value Ҫ���ӵĴ�С
     * @return �޸�ֵ�󣬸����ƶ���ĵ�ַ
     */
    memory_chunk* increase_element_value(memory_chunk* chunk, size_t increase_value);

    /**
     * ָ�����chunksize��С��С
     * @param chunk Ҫ�Ķ�����
     * @param decrease_value Ҫ��С�Ĵ�С
     * @return �޸�ֵ�󣬸����ƶ���ĵ�ַ
     */
    memory_chunk* decrease_element_value(memory_chunk* chunk, size_t decrease_value);

    public:
    memory_chunk*       m_pHeap;        //< �󶥶���ʼ��ַ
    size_t              m_nMaxSize;     //< ������     
    size_t              m_nCurrentSize; //< ��ǰ����  
};

