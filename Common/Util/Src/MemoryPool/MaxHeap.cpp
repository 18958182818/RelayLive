#include "stdafx.h"
#include "Maxheap.h"


CMaxHeap::CMaxHeap(void)
{
}


CMaxHeap::~CMaxHeap(void)
{
}
                                                                       
void CMaxHeap::init_max_heap(size_t max_heap_size, memory_chunk* heap_arr)  
{  
    m_nMaxSize = max_heap_size;  
    m_nCurrentSize = 0;  
    m_pHeap = heap_arr;  
} 

bool CMaxHeap::is_heap_empty()  
{  
    return m_nCurrentSize == 0;    
}

bool CMaxHeap::is_heap_full()  
{  
    return m_nCurrentSize == m_nMaxSize;    
}

memory_chunk* CMaxHeap::filter_up(size_t start)  
{  
    if (start < 0 || start >= m_nCurrentSize)
    {
        Log::error("filter_up failed, start:%d,m_nCurrentSize:%d",start,m_nCurrentSize);
        return nullptr;
    }

    size_t i = start;
    size_t j = ( i - 1 ) / 2;   // ���㸸�ڵ�λ��
    memory_chunk temp = m_pHeap[i];
    while(i > 0)
    {    
        if(temp.chunk_size <= m_pHeap[j].chunk_size)
            break;    
        else  
        {             
            m_pHeap[i] = m_pHeap[j];    // ���㽻�������������ڵ㿽����Ҷ�ӽڵ�
            if(m_pHeap[j].pfree_mem_addr == nullptr)
            {
                Log::error("m_pHeap[j=%d].pfree_mem_addr is null",j);
                return nullptr;
            }
            m_pHeap[j].pfree_mem_addr->pmem_chunk = &(m_pHeap[i]);

            i = j;    // ����һ������
            j = (i - 1) / 2;    
        }    
    }
    // ���������Ľڵ㣬������λ�õ�ֵ���������������
    m_pHeap[i] = temp;

    // ���ص����ƶ�λ�ú�ĵ�ַ
    // �˴���û�н�tempָ���block����ָ��m_pHeap[i]��������������洦��
    // ������������õ��������:
    // 1��ɾ���ýڵ㣬���Ӧ��blockָ���chunk��ĳ�null��
    // 2�����Ӵ�С������increase_element_value��blockָ���chunk���ó��䷵��ֵ
    // 3�����ӽڵ㣬����insert_heap�󣬽�blockָ���chunk���ó��䷵��ֵ
    return &(m_pHeap[i]);
}

memory_chunk* CMaxHeap::filter_down(size_t start)  
{
    if (start < 0 || start >= m_nCurrentSize)
    {
        Log::error("filter_up failed, start:%d,m_nCurrentSize:%d",start,m_nCurrentSize);
        return nullptr;
    }
    size_t i = start;  
    size_t j = i * 2 + 1;   // ��������ӵ�λ�ã�j+1�����Ҷ��ӵ�λ��
    size_t endOfHeap = m_nCurrentSize-1;
    memory_chunk temp = m_pHeap[i];
    while(j <= endOfHeap)  
    {
        // j��j+1��i������Ҷ�ӽڵ㣬ȡ���һ��
        if(j < endOfHeap && m_pHeap[j].chunk_size < m_pHeap[j+1].chunk_size)
            j++;

        if(temp.chunk_size > m_pHeap[j].chunk_size)
            break;
        else  
        {
            m_pHeap[i] = m_pHeap[j];
            if(m_pHeap[j].pfree_mem_addr == nullptr)
            {
                Log::error("m_pHeap[j].pfree_mem_addr is null");
                return nullptr;
            }
            m_pHeap[j].pfree_mem_addr->pmem_chunk = &(m_pHeap[i]);
            i = j; 
            j = 2 * i + 1;
        }
    }
    // ���������Ľڵ㣬����һ��������ֵ���������������
    m_pHeap[i] = temp;  

    // ���ص����ƶ�λ�ú�ĵ�ַ
    // �˴���û�н�tempָ���block����ָ��m_pHeap[i]��������������洦��
    // ������������õ��������:
    // 1��ɾ���Ѷ��ڵ㣬������Ҷ�Ӹ��ǶѶ���������remove_max��Ϊ����Ҷ�Ӷ�Ӧ��block����ָ���chunk����λ��
    // 2�Ǽ�С��С������decrease_element_value��blockָ���chunk���ó��䷵��ֵ
    return &(m_pHeap[i]);
}

memory_chunk* CMaxHeap::insert_heap(memory_chunk& chunk)
{
    if (is_heap_full())
    {
        Log::error("heap is full");
        return nullptr;
    }
    m_pHeap[m_nCurrentSize] = chunk;  
    ++m_nCurrentSize;
    memory_chunk* ret = filter_up(m_nCurrentSize-1); 
    return ret;
}

bool CMaxHeap::get_max(memory_chunk*& chunk)  
{  
    if(is_heap_empty())  
    {    
        return false;    
    }    
    chunk = m_pHeap;    
    return true;  
}

bool CMaxHeap::remove_max()  
{  
    if(is_heap_empty())  
    {    
        return false;    
    }

    if (m_nCurrentSize > 1)  
    {
        //�����һ�����ǶѶ���Ȼ������������
        m_pHeap[0] = m_pHeap[m_nCurrentSize - 1];
        --m_nCurrentSize; // chunk����1
        // ԭ�ȵĶѶ�ָ���block�����������pmem_chunk��Ϊnull�����¶Ѷ���ԭ�����һ��Ҷ�ӣ�����Ӧ��block��pmem_chunkҪ����
        memory_block* pBlock = m_pHeap[0].pfree_mem_addr;  
        memory_chunk* pTmp = filter_down(0);
        pBlock->pmem_chunk = pTmp;    
    }
    else // m_nCurrentSize == 1
    {
        --m_nCurrentSize; // chunk����1
    }
    return true;    
}

void CMaxHeap::remove_element(memory_chunk* chunk)  
{   
    // ����Ԫ��size������󣨴���max element��
    chunk->chunk_size = m_nMaxSize + 1;  
    // Ȼ�����������Ѷ�����chunk��Ӧ��block��pmem_chunk���õĵط���Ϊnull
    filter_up(chunk - m_pHeap);
    // ɾ���Ѷ�Ԫ��     
    remove_max();  
}

memory_chunk* CMaxHeap::increase_element_value(memory_chunk* chunk, size_t increase_value)  
{  
    size_t index = chunk - m_pHeap;  
    chunk->chunk_size += increase_value;  
    return filter_up(index);  
}

memory_chunk* CMaxHeap::decrease_element_value(memory_chunk* chunk, size_t decrease_value)  
{
    size_t index = chunk - m_pHeap;  
    chunk->chunk_size -= decrease_value;  
    return filter_down(index);  
}  