#include "stdafx.h"
#include "Mutex.h"
#include "MemoryPoolMaxheap.h"
#include "MaxHeap.h"


CMemoryPoolMaxheap::CMemoryPoolMaxheap(void)
{
}


CMemoryPoolMaxheap::~CMemoryPoolMaxheap(void)
{
    clear();
}

bool CMemoryPoolMaxheap::init(int nUnitCount, int nUnitSize)
{
    if(!IMemoryPool::init(nUnitCount,nUnitSize))
        return false;
    MutexLock  lock(&m_cs);

    // �����󶥶�
    CMaxHeap* pMaxHeap = new CMaxHeap;
    if(pMaxHeap == nullptr) return false;
    m_pMaxHeap = (void*)pMaxHeap;

    // ��ʼ���󶥶ѣ�chunk pool�Ǵ󶥶���ʽ
    pMaxHeap->init_max_heap(m_nUnitCount, (memory_chunk*)m_pChunkPool);

    // ��ʼ�� chunk pool
    memory_chunk chunk;  
    chunk.chunk_size = m_nUnitCount;
    memory_chunk* pos = pMaxHeap->insert_heap(chunk);
    if (nullptr == pos)
    {
        Log::error("init insert failed");
        return false;
    }
    // ��ʼchunkָ����ʼmemory block
    pos->pfree_mem_addr = m_pMemoryMapTable;

    // ��ʼ�� memory map table  
    m_pMemoryMapTable[0].count = m_nUnitCount;  
    m_pMemoryMapTable[0].pmem_chunk = pos;  
    m_pMemoryMapTable[m_nUnitCount-1].start = 0;  

    m_nUsedSize = 0;

    m_bInit = true;
    return true;
}

void CMemoryPoolMaxheap::clear()
{
    if (m_bInit == false)
        return;
    MutexLock  lock(&m_cs);

    if(nullptr != m_pMemoryMapTable)
    {
        delete m_pMemoryMapTable;
        m_pMemoryMapTable = nullptr;
    }

    if (nullptr != m_pMaxHeap)
    {
        delete m_pMaxHeap;
        m_pMaxHeap = nullptr;
    }

    m_bInit = false;
}

void* CMemoryPoolMaxheap::mp_malloc(size_t nSize)  
{
    if(!m_bInit || m_pChunkPool == nullptr)
        return nullptr;

    // ��Ҫ�����block����
    size_t count = (nSize + m_nUnitSize - 1) / m_nUnitSize;
    // ��Ҫ����Ĵ�С
    size_t sMemorySize = count * m_nUnitSize;

    CMaxHeap* pMaxHeap = (CMaxHeap*)m_pMaxHeap;

    MutexLock  lock(&m_cs);
    // ��ȡ�Ѷ�
    memory_chunk* max_chunk = nullptr;  
    bool ret = pMaxHeap->get_max(max_chunk);
    if (nullptr == max_chunk || nullptr == max_chunk->pfree_mem_addr)
    {
        Log::error("null ptr max_chunk:%d",max_chunk);
        return nullptr;  
    }
    if (ret == false || max_chunk->chunk_size < count)  
    {
        Log::error("there is not enough memory in max chunk ret:%d,max chunk_size:%d,count:%d",ret,max_chunk->chunk_size,count);
        return nullptr;  
    } 

    m_nUsedSize += sMemorySize;
    m_nUsedCount += count;
    Log_Debug("mp_malloc UsedCount:%d/%d, UsedSize:%d,requestSize:%d/%d,requestCount:%d/%d", m_nUsedCount,m_nUnitCount,m_nUsedSize,nSize,sMemorySize,count,max_chunk->chunk_size);

    if (max_chunk->chunk_size == count)  
    {
        // ��Ҫ������ڴ��С�뵱ǰchunk�е��ڴ��С��ͬʱ���Ӷ���ɾ����chunk  
        size_t current_index = (max_chunk->pfree_mem_addr - m_pMemoryMapTable);  
         max_chunk->pfree_mem_addr->pmem_chunk = nullptr; 
        if(!pMaxHeap->remove_max())
        {
            Log::error("mp_malloc remove_max failed");
            return nullptr;
        }

        Log_Debug("mp_malloc end");
        return index2addr(current_index);  
    }  
    else  // max_chunk->chunk_size > count
    {  
        // ��Ҫ������ڴ�С�ڵ�ǰchunk�е��ڴ�ʱ�����Ķ�����Ӧchunk��pfree_mem_addr  

        // ��¼ԭ�ȵ�����block����
        size_t old_block_count = max_chunk->pfree_mem_addr->count;

        // �Ѷ�chunkָ���block��current_block�Ƿ���ʹ�õ���ʼblock
        memory_block* current_block = max_chunk->pfree_mem_addr;
        // ����ʹ�õ�block����
        current_block->count = count;
        // �����������ʼblock��λ��
        size_t current_index = current_block - m_pMemoryMapTable;
        // ��������Ľ���block
        m_pMemoryMapTable[current_index+count-1].start = current_index; 
        // NULL��ʾ��ǰ�ڴ���ѱ�����  
        current_block->pmem_chunk = nullptr; 
        Log_Debug("malloc current_index:%d,addr:%d",current_index,current_block);

        // chunkԭ����block���������õ��ĸ��������Ǹ�chunkʣ�µ�block��
        memory_chunk* pos = pMaxHeap->decrease_element_value(max_chunk, count);
        if (nullptr == pos)
        {
            Log::error("new chunk pos is null");
            return nullptr;
        }

        // �����ʣ�����ʼblock
        memory_block& new_first = m_pMemoryMapTable[current_index+count];
        // ʣ��block�ĸ���
        new_first.count = old_block_count - count;  
        // �����ʣ��Ľ���block��ָ��ʣ�����ʼλ��
        size_t end_index = current_index + old_block_count - 1;  
        m_pMemoryMapTable[end_index].start = current_index + count; 
        // ʣ��blockָ���chunk
        new_first.pmem_chunk = pos; 

        // ����chunkָ���block
        pos->pfree_mem_addr = &new_first;  

        Log_Debug("mp_malloc end");
        return index2addr(current_index);  
    }
}

void CMemoryPoolMaxheap::mp_free(void* p)   
{
    if(!m_bInit || m_pChunkPool == nullptr)
        return;
    MutexLock  lock(&m_cs);

    CMaxHeap* pMaxHeap = (CMaxHeap*)m_pMaxHeap;
    size_t current_index = addr2index(p);
    if (current_index >= m_nUnitCount)
    {
        Log::error("free index is bigger than max %d/%d",current_index,m_nUnitCount);
        return;
    }
    
    // ��ӡ��Ϣ
    memory_block& blockTmp = m_pMemoryMapTable[current_index];
    Log_Debug("mp_free current_index:%d,blockInfo addr:%d,count:%d,pChunk:%d,start:%d",current_index,&blockTmp,blockTmp.count,blockTmp.pmem_chunk,blockTmp.start);
 
    // Ҫ�ͷŵĴ�С
    size_t count = m_pMemoryMapTable[current_index].count;
    size_t size =  count* m_nUnitSize;  
    m_nUsedCount -= count;
    m_nUsedSize -= size;
    Log_Debug("mp_free UsedCount:%d/%d, UsedSize:%d,freeCount:%d", m_nUsedCount,m_nUnitCount,m_nUsedSize,count);
    if (current_index + count > m_nUnitCount)
    {
        Log::error("free chunk end is bigger than max %d,%d/%d",current_index,count,m_nUnitCount);
        return;
    }

    // �ж��뵱ǰ�ͷŵ��ڴ�����ڵ��ڴ���Ƿ�����뵱ǰ�ͷŵ��ڴ��ϲ�
    memory_block* pre_block = nullptr;  // ǰһ����
    memory_block* next_block = nullptr;  // ��һ����
    memory_block* current_block = &(m_pMemoryMapTable[current_index]);  // �ͷŵĵ�ǰ��
    // ��һ��
    if (current_index == 0)
    {
        if (count < m_nUnitCount)  
        {
            // ��һ����
            next_block = &(m_pMemoryMapTable[current_index+count]);  
            // �����һ���ڴ���ǿ��еģ��ϲ�  
            if (next_block->pmem_chunk != nullptr)  
            {
                // ��һ��chunk�Ĵ�С���ӣ�����������
                memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)(next_block->pmem_chunk), count);  
                pos->pfree_mem_addr = current_block;
                // ��һ����ָ��Ŀ�ͷ�Ƶ��ͷŵĿ鿪ͷ
                m_pMemoryMapTable[current_index+count+next_block->count-1].start = current_index;  
                current_block->count += next_block->count;  
                current_block->pmem_chunk = pos;  
                next_block->pmem_chunk = nullptr;  // �����ǿ鿪ͷ
            }  
            // �����һ���ڴ治�ǿ��еģ���pfree_mem_chunk������һ��chunk  
            else  
            {
                memory_chunk new_chunk;  
                new_chunk.chunk_size = current_block->count;  
                new_chunk.pfree_mem_addr = current_block;  
                memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);
                if(pos == nullptr) 
                {
                    Log::error("insert_heap failed");
                    return;
                }
                pos->pfree_mem_addr = current_block;
                current_block->pmem_chunk = pos;  
            }  
        }  
        else  // current_block->count == m_nUnitCount  current_index == 0
        {
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);  
            if(pos == nullptr) 
            {
                Log::error("insert_heap failed");
                return;
            }
            pos->pfree_mem_addr = current_block;
            current_block->pmem_chunk = pos;  
        }         
    }  
    // ���һ��  
    else if (current_index + count == m_nUnitCount)  
    {
        if (count < m_nUnitCount)  // current_index > 0
        {
            // �ҵ�ǰһ����
            pre_block = &(m_pMemoryMapTable[current_index-1]); 
            size_t index = pre_block->start;  
            pre_block = &(m_pMemoryMapTable[index]);
            Log_Debug("current_index:%d,index:%d,pre_block->pmem_chunk:%d",current_index,index,pre_block->pmem_chunk);

            // ���ǰһ���ڴ���ǿ��еģ��ϲ�  
            if (pre_block->pmem_chunk != nullptr)  
            {
                memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)pre_block->pmem_chunk, count);  
                pos->pfree_mem_addr = pre_block;
                pre_block->pmem_chunk = pos;  
                // �ͷŵĿ�Ľ�βָ��ͷ
                m_pMemoryMapTable[current_index+count-1].start = current_index - pre_block->count;  
                pre_block->count += count;  
                current_block->pmem_chunk = nullptr;  // ������chunk��ͷ
            }  
            // ���ǰһ���ڴ治�ǿ��еģ���pfree_mem_chunk������һ��chunk  
            else  
            {
                memory_chunk new_chunk;  
                new_chunk.chunk_size = current_block->count;  
                new_chunk.pfree_mem_addr = current_block;  
                memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);  
                current_block->pmem_chunk = pos;  
            }  
        }  
        else  // current_block->count == m_nUnitCount  current_index == 0
        {
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);  
            if(pos == nullptr) 
            {
                Log::error("insert_heap failed");
                return;
            }
            pos->pfree_mem_addr = current_block;
            current_block->pmem_chunk = pos;  
        }  
    }  
    else // ���ǿ�ͷ����β�� 
    {
        //��һ����
        next_block = &(m_pMemoryMapTable[current_index+count]);
        Log_Debug("next block:%d,count:%d",next_block,next_block->count);
        //ǰһ����
        pre_block = &(m_pMemoryMapTable[current_index-1]);  
        size_t index = pre_block->start;  
        pre_block = &(m_pMemoryMapTable[index]);
        Log_Debug("pre block:%d,index:%d,count:%d",pre_block,index,pre_block->count);
        //�Ƿ����һ����ϲ�
        bool is_back_merge = false;  
        // ǰ��Ŀ鶼��ʹ�ã�ֱ������chunk
        if (next_block->pmem_chunk == nullptr && pre_block->pmem_chunk == nullptr)  
        {
            memory_chunk new_chunk;  
            new_chunk.chunk_size = current_block->count;  
            new_chunk.pfree_mem_addr = current_block;  
            memory_chunk* pos = pMaxHeap->insert_heap(new_chunk);
            pos->pfree_mem_addr = current_block;
            current_block->pmem_chunk = pos;  
        }  
        // ��һ���ڴ��δʹ��
        if (next_block->pmem_chunk != nullptr)  
        {
            Log_Debug("mp_free bb  next_block->pmem_chunk:%d",next_block->pmem_chunk);
            // ��һ��λ�õ�chunk���Ӵ�С
            memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)next_block->pmem_chunk, current_block->count);  
            pos->pfree_mem_addr = current_block; 
            //��һ�����ָ��
            m_pMemoryMapTable[current_index+current_block->count+next_block->count-1].start = current_index;
            //�ϲ���С
            current_block->count += next_block->count;  
            current_block->pmem_chunk = pos;  
            next_block->pmem_chunk = nullptr;  // next block ��������ʼ
            is_back_merge = true;  
        }  
        // ǰһ���ڴ��δʹ��
        if (pre_block->pmem_chunk != nullptr)  
        {
            // ���ĩβblockָ��ǰһ���鿪ͷ
            m_pMemoryMapTable[current_index+current_block->count-1].start = current_index - pre_block->count;
            // ǰһ�������Ӵ�С
            pre_block->count += current_block->count;
            // ǰһ�����Ӧ��chunk���Ӵ�С
            memory_chunk* pos = pMaxHeap->increase_element_value((memory_chunk*)pre_block->pmem_chunk, current_block->count);  
            pre_block->pmem_chunk = pos;  
            pos->pfree_mem_addr = pre_block;  
            if (is_back_merge)  
            {
                // ֮ǰ���ϲ����ģ���Ҫ�Ƴ�һ��chunk
                pMaxHeap->remove_element((memory_chunk*)current_block->pmem_chunk);  
            }  
            current_block->pmem_chunk = nullptr;    // ��������ʼblock          
        }     
    } 
    Log_Debug("mp_free end");
}

size_t CMemoryPoolMaxheap::get_chunk_pool_size()
{
    return sizeof(memory_chunk) * m_nUnitCount;
}

void CMemoryPoolMaxheap::showinfo()
{
    for (int i=0; i<m_nUnitCount; ++i)
    {
        memory_chunk* pPool = (memory_chunk*)m_pChunkPool;
        memory_chunk* pChunk = (memory_chunk*)m_pMemoryMapTable[i].pmem_chunk;
        Log_Debug("block info:%d,count:%d,start:%d,pchunk:%d,index:%d"
            ,i,m_pMemoryMapTable[i].count,m_pMemoryMapTable[i].start,pChunk
            ,pChunk?pChunk-pPool:-1);
    }
    for (int i=0; i<m_nUnitCount; ++i)
    {
        memory_chunk* pPool = (memory_chunk*)m_pChunkPool;
        Log_Debug("chunk info:%d,size:%d,pblock:%d,index:%d",i,pPool[i].chunk_size,pPool[i].pfree_mem_addr,pPool[i].pfree_mem_addr-m_pMemoryMapTable);
    }
    CMaxHeap* pMaxHeap = (CMaxHeap*)m_pMaxHeap;
    Log_Debug("chunk current Size:%d",pMaxHeap->m_nCurrentSize);
}