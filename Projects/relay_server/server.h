#pragma once
#include <string>
#include <stdint.h>

class ILiveSession {
public:
    virtual void AsyncSend() = 0;
};

/**
 * uri�н���������
 */
struct RequestParam {
    std::string           strUrl;               // ԭʼ��Ƶ��ַ��������
    std::string           strType;              // ý�����ͣ�Ĭ��Ϊflv
    uint32_t              nWidth;               // ��Ƶ��ȣ�Ĭ��Ϊ0����������Ƶ
    uint32_t              nHeight;              // ��Ƶ�߶ȣ�Ĭ��Ϊ0����������Ƶ
    uint32_t              nProbSize;            // ̽��PS���Ĵ�С��Ĭ��Ϊ25600
    uint32_t              nProbTime;            // ̽��PS����ʱ�䣬Ĭ��Ϊ1��
};

namespace Server {

int Init(int port);

int Cleanup();
};