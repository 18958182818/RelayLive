#pragma once
#include "ExportDefine.h"

#include <windows.h> 

/**
 * �߾��ȼ�ʱ�������ڼ���һ�����������ĵ�ʱ��
 */
class Clock
{
public:
    Clock()
    {
    }
    
    /** ��ü�������ʱ��Ƶ�� */
    static double prequency()
    {
        static double m_dfFreq = -1;
        if(m_dfFreq <= 0)
        {
            LARGE_INTEGER litmp; 
            QueryPerformanceFrequency(&litmp);  
            m_dfFreq = (double)litmp.QuadPart;
        }
        return m_dfFreq;
    }

    /** ������ʱ�� */
    void start()
    {
        LARGE_INTEGER litmp; 
        QueryPerformanceCounter(&litmp);  
        m_llStart = litmp.QuadPart;
    }

    /** ֹͣ��ʱ�� */
    void end()
    {
        LARGE_INTEGER litmp; 
        QueryPerformanceCounter(&litmp);  
        m_llEnd = litmp.QuadPart;
    }

    /**
     * ��ȡstart��end֮���ʱ����
     * @return ����ļ�����������Ҫ���Լ�������ʱ��Ƶ�ʲ��Ǻ���
     * @note ���start��end����϶̣�get��ֵ̫С��Ӧ�ý����get�Ľ���������ٽ��г����㣬����᲻׼
     */
    LONGLONG get()
    {
        return (m_llEnd - m_llStart);
    }
private:
    LONGLONG    m_llStart;
    LONGLONG    m_llEnd;
    
};