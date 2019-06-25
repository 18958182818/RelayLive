#pragma once
#ifdef USE_FFMPEG
#include "avtypes.h"
#include "ring_buff.h"
#include "uv.h"
#include "Netstreammaker.h"

enum NalType;
namespace LiveClient
{
    class IDecoder
    {
    public:
        IDecoder(){};
        virtual ~IDecoder(){};

        static IDecoder* Create(AV_CALLBACK cb, void* handle=NULL);

        virtual int Decode(AV_BUFF buff) = 0;
    };

    class IEncoder
    {
    public:
        IEncoder(){};
        virtual ~IEncoder(){};
        static IEncoder* Create(AV_CALLBACK cb, void* handle=NULL);
        virtual int Code(AV_BUFF buff) = 0;
        virtual void SetDecoder(IDecoder* dec) = 0;
    };
};
#endif