#pragma once
#include "commonDefine.h"

/**
 * �̳߳��еĹ����߳�
 */
class Runnable
{
public:
    virtual ~Runnable() {}
    virtual void run() = 0;
};

/**
 * IOCPģ���е�I/O�߳�
 */
class IoRunnable
{
public:
    virtual ~IoRunnable() {}
    virtual void run(OVERLAPPED *o, ulong ioResult, ulong64 bytes, PTP_IO io) = 0;
};
