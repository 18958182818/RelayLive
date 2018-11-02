#include "stdafx.h"
#include "ChunkList.h"

CChunkList::CChunkList(void)
    : m_pHead(nullptr)
    , m_nCount(0)
{
}

CChunkList::~CChunkList(void)
{
}

memory_chunk* CChunkList::pop_front()  
{  
    if (nullptr == m_pHead)  // �����ѿ�
    {  
        return nullptr;  
    }

    memory_chunk* tmp = m_pHead;  
    m_pHead = m_pHead->next;
    if(m_pHead)
        m_pHead->pre = nullptr; 
    --m_nCount;
    return  tmp;  
}

void CChunkList::push_front(memory_chunk* element)  
{
    if (nullptr == element)
    {
        return;
    }

    element->pre = nullptr;
    element->next = m_pHead;
    if (m_pHead != nullptr)
    { 
        m_pHead->pre = element;   
    }
    ++m_nCount;
    m_pHead = element;
}

void CChunkList::delete_chunk(memory_chunk* element)  
{ 
    if (element == nullptr)  
    {  
        return;  
    }  
    // elementΪ����ͷ  
    else if (element == m_pHead)  
    {
        // ����ֻ��һ��Ԫ��  
        m_pHead = m_pHead->next;
        if(m_pHead)
            m_pHead->pre = nullptr;   
    }  
    // elementΪ����β  
    else if (element->next == nullptr)  
    {
        element->pre->next = nullptr;  
    }  
    else  
    {
        element->pre->next = element->next;  
        element->next->pre = element->pre;  
    }

    element->pre = nullptr;  
    element->next = nullptr;  
    --m_nCount;
}