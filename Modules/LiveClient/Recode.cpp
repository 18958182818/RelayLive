#include "stdafx.h"
#include "Recode.h"
#ifdef USE_FFMPEG
#include "utilc.h"
#include "es.h"
#include "h264.h"
#include "flv.h"

extern "C"
{
#define __STDC_FORMAT_MACROS
#define snprintf  _snprintf
    //#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"  
    //#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"
    //#include "libavutil/timestamp.h"
}
#pragma comment(lib,"avcodec.lib")
//#pragma comment(lib,"avdevice.lib")
//#pragma comment(lib,"avfilter.lib")
//#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
//#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")

namespace LiveClient
{
    typedef struct _H264_BUFF_ {
        NalType   eType;    //< ���ݰ�����
        char      *pData;   //< ��������
        uint32_t  nLen;     //< ���ݳ���
    }H264_BUFF;

    static void H264SpsCbfun(uint32_t nWidth, uint32_t nHeight, double fFps, void* pUser);
    static void RecodeCallback(AV_BUFF buff, void* pUser);

    /**
    * h264����
    * ��������������parser������ÿ��������һ֡���ݴ�ŵ�pkt��
    * ��pkt�����ݽ���decode��yuv������������frame
    */
    class CDecoder {
    public:
        AVCodec              *codec;     //h264�������
        AVCodecContext       *c;         //����������
        AVCodecParserContext *parser;    //h264������
        AVFrame              *frame;     //ԭʼ���ݽ�����yuvͼƬ
        AVPacket             pkt;        //���������h264����
        bool                 ok;
        //FILE                 *f;
        CDecoder()
            : ok(false){
                //�������ʵ��
                codec = avcodec_find_decoder(AV_CODEC_ID_H264); 
                if (!codec){
                    Log::error("Codec not found\n");
                    return;
                }

                //������������ģ�ָ���˱����Ĳ���
                c = avcodec_alloc_context3(codec); //����AVCodecContextʵ��
                if (!c) {
                    Log::error("Could not allocate video codec context\n");
                    return;
                }

                //�������������������н�ȡ������һ��NAL Unit����
                parser = av_parser_init(AV_CODEC_ID_H264);
                if (!parser){
                    Log::error("Could not allocate video parser context\n");
                    return;
                }

                //��AVCodec����
                int ret = avcodec_open2(c, codec, NULL);
                if (ret < 0){
                    char tmp[1024]={0};
                    av_strerror(ret, tmp, 1024);
                    Log::error("Could not open codec %d:%s", ret, tmp);
                    return;
                }

                //��װͼ�����ָ��
                frame = av_frame_alloc();
                if (!frame) {
                    Log::error("Could not allocate video frame");
                    return;
                }

                //��װ��������ʵ��
                av_init_packet(&pkt);
                pkt.data = NULL;
                pkt.size = 0;

                ok = true;
                //f = fopen("test.yuv", "wb");
        }

        ~CDecoder(){
            avcodec_close(c);
            av_parser_close(parser);
            av_free(c);
            av_frame_free(&frame);
            AVPacket *p = &pkt;
            av_packet_free(&p);
        }
    };

    /**
    * h264����
    */
    class CEncoder {
    public:
        SwsContext      *swsc;      //����������
        AVCodec         *codec;     //h264�������
        AVCodecContext  *c;         //����������
        AVFrame         *frame;     //���ź��yuvͼƬ
        AVPacket        pkt;        //������h264����
        uint32_t        width;      //���Ŵ�С
        uint32_t        height;     //���Ŵ�С
        CES             *pEs;       // ES�����������nalu
        CH264           *pH264;     // H264������
        CFlv            *pFlv;      // FLV�����
        //uint64_t        pts;        // ��¼PES�е�pts
        //uint64_t        dts;        // ��¼PES�е�dts
        AV_CALLBACK     funCB;
		void            *user;
        bool            ok;
        //FILE            *yuv;
        //FILE            *f;
        //FILE            *flv;
        CEncoder()
            : ok(false)
            //, pts(0)
            //, dts(0)
        {
            pEs            = new CES(RecodeCallback, this);
            pH264          = new CH264(H264SpsCbfun, RecodeCallback, this);
            pFlv           = new CFlv(RecodeCallback, this); 

            //��װͼ�����ָ��
            frame = av_frame_alloc();
            if (!frame) {
                Log::error("Could not allocate video frame");
                return;
            }

            //��װ��������ʵ��
            av_init_packet(&pkt);
            pkt.data = NULL;
            pkt.size = 0;

            //yuv = fopen("test2.yuv", "wb");
            //f = fopen("test.264", "wb");
            //flv = fopen("test.flv", "wb");
        }

        ~CEncoder(){
            avcodec_close(c);
            av_free(c);
            av_frame_free(&frame);
            AVPacket *p = &pkt;
            av_packet_free(&p);
            sws_freeContext(swsc);
            SAFE_DELETE(pEs);
            SAFE_DELETE(pH264);
            SAFE_DELETE(pFlv);
        }

        void Start(CDecoder *decode){
            //����������
            swsc = sws_getContext(decode->frame->width
                , decode->frame->height
                , (AVPixelFormat)decode->frame->format
                , width, height
                , (AVPixelFormat)decode->frame->format
                , SWS_BICUBIC , NULL, NULL, NULL);

            if(swsc == NULL){
                Log::error("sws_getContext failed");
                return;
            }

            //�������ʵ��ָ��
            codec = avcodec_find_encoder(AV_CODEC_ID_H264); 
            if (!codec){
                Log::error("Codec not found\n");
                return;
            }

            //������������ģ�ָ���˱����Ĳ���
            c = avcodec_alloc_context3(codec); //����AVCodecContextʵ��
            if (!c) {
                Log::error("Could not allocate video codec context\n");
                return;
            }
            c->bit_rate = decode->c->bit_rate; //4000000;
            c->width    = width;
            c->height   = height;
            c->time_base.num = 1;
            c->time_base.den = 25;
            c->pix_fmt  = (enum AVPixelFormat)decode->frame->format;
            c->gop_size = 15;
            c->has_b_frames = 0;
            c->max_b_frames = 0;
            c->qmin = 34;
            c->qmax = 50;
            frame->format = (enum AVPixelFormat)decode->frame->format;
            frame->width = width;
            frame->height = height;

            // �����������룬����ʱ
            AVDictionary *options = NULL;
            av_dict_set(&options, "preset", "superfast",   0);
            //av_dict_set(&options, "tune",   "zerolatency", 0);

            //��AVCodec����
            if (avcodec_open2(c, codec, &options) < 0){
                Log::error("Could not open codec");
                return;
            }

            int size = av_image_alloc(frame->data, frame->linesize, width, height, (enum AVPixelFormat)(decode->frame->format), 16);
            Log::debug("image size %d", size);

			pFlv->SetSps(width, height, 25);

            ok= true;
        }

        void push_h264_stream(AV_BUFF buff)
        {
            //nal_unit_header4* nalu = (nal_unit_header4*)pBuff;
            //Log::debug("ESParseCb nlen:%ld, buff:%02X %02X %02X %02X %02X", nLen,pBuff[0],pBuff[1],pBuff[2],pBuff[3],pBuff[4]);
            CHECK_POINT_VOID(buff.pData);
            pH264->InputBuffer(buff.pData, buff.nLen);
            NalType m_nalu_type = pH264->NaluType();
            uint32_t nDataLen = 0;
            char* pData = pH264->DataBuff(nDataLen);
            pFlv->Code(m_nalu_type, pData, nDataLen);
        }
    };

    static void H264SpsCbfun(uint32_t nWidth, uint32_t nHeight, double fFps, void* pUser){
        CEncoder* pLive = (CEncoder*)pUser;
        //pLive->set_h264_param(nWidth, nHeight, fFps);
    }

    static void RecodeCallback(AV_BUFF buff, void* pUser) {
        CEncoder* encoder = (CEncoder*)pUser;
        switch (buff.eType)
        {
        case AV_TYPE::H264_NALU:
            {
                encoder->push_h264_stream(buff);
            }
            break;
        case AV_TYPE::FLV_HEAD:
        case AV_TYPE::FLV_FRAG_KEY:
        case AV_TYPE::FLV_FRAG:
            {
                //Log::debug("flv %d", buff.eType);
				encoder->funCB(buff, encoder->user);
                //fwrite(buff.pData, 1, buff.nLen, encoder->flv);
                //fflush(encoder->flv);
            }
            break;
        }
    }

    static void recode_thread(void* arg)
    {
        CRecoder* h = (CRecoder*)arg;
        h->RecodeThread();
    }

    CRecoder::CRecoder(void* handle)
        : m_hUser(handle)
        , m_decode(nullptr)
        , m_codec(nullptr)
        , m_bFirstKey(false)
        , m_bGotSPS(false)
        , m_bGotPPS(false)
        , m_timestamp(0)
        , m_tick_gap(3600)
        , m_bRun(true)
        , m_bFinish(false)
    {
        m_decode = new CDecoder;
        m_codec = new CEncoder;

        m_pSPS = new CFlvStreamMaker();
        m_pPPS = new CFlvStreamMaker();
        m_pKeyFrame = new CFlvStreamMaker();

        m_pRingH264 = create_ring_buff(sizeof(H264_BUFF), 1000, NULL);

        uv_thread_t tid;
        uv_thread_create(&tid, recode_thread, this);
    }


    CRecoder::~CRecoder(void)
    {
        m_bRun = false;
        destroy_ring_buff(m_pRingH264);
		while(!m_bFinish){
			Sleep(10);
		}
        Log::debug("CReCode release");
    }

    void CRecoder::SetChannel1(uint32_t nWidth, uint32_t nHeight, AV_CALLBACK cb)
    {
        m_codec->width = nWidth;
        m_codec->height = nHeight;
        m_codec->funCB = cb;
		m_codec->user = m_hUser;
    }

    int CRecoder::Recode(AV_BUFF buff, NalType t)
    {
		return Recode2(buff, t);

        // �����ݱ�����ring buff
        int n = (int)ring_get_count_free_elements(m_pRingH264);
        if (!n) {
            Log::error("rtp ring buff is full");
            return -1;
        }

        H264_BUFF newTag = {t, buff.pData, buff.nLen};
        if (!ring_insert(m_pRingH264, &newTag, 1)) {
            Log::error("add data to rtp ring buff failed");
            return -1;
        }

        return 0;
    }

    int CRecoder::Recode2(AV_BUFF buff, NalType t)
    {
        if(!m_decode->ok){
            Log::error("decode is not ok");
            return -1;
        }
        //Log::debug("begin recode %lld  %lld", pts, dts);
        //Log::debug("begin recode %d", t);

        switch (t)
        {
        case b_Nal:  // �ǹؼ�֡
            {
                if(!m_bFirstKey)
                    break;
                //������ڹؼ�֡û�д�����Ҫ�ȷ��͹ؼ�֡��pps�����ڹؼ�֡������յ���
                //sps��pps��ʧ������ʹ����һ�εġ�
                EncodeKeyVideo(); 
                //Log::debug("send frame");
                EncodeVideo(buff.pData, buff.nLen,0);
                m_timestamp += m_tick_gap;
            }
            break;
        case idr_Nal: // �ؼ�֡
            {
                m_pKeyFrame->clear();
                m_pKeyFrame->append_data(buff.pData, buff.nLen);
                //һ��sps��pps���ڹؼ�֡ǰ�棬����ʱ���ڹؼ�֡���档
                if(m_bGotPPS && m_bGotSPS)
                    EncodeKeyVideo();
            }
            break;
        case sei_Nal:
            break;
        case sps_Nal:
            {
                //Log::debug("save sps size:%d",nLen);
                CHECK_POINT_INT(m_pSPS,-1);
                m_pSPS->clear();
                m_pSPS->append_data(buff.pData, buff.nLen);
                m_bGotSPS = true;
            }
            break;
        case pps_Nal:
            {
                //Log::debug("save pps size:%d",nLen);
                CHECK_POINT_INT(m_pPPS,-1);
                m_pPPS->clear();
                m_pPPS->append_data(buff.pData, buff.nLen);
                m_bGotPPS = true;
            }
            break;
        case other:
        case unknow:
        default:
            Log::warning("h264 nal type: %d", t);
            break;
        }

        return 0;
    }

    void CRecoder::RecodeThread()
    {
        H264_BUFF buff;
        while (m_bRun)
        {
            int ret = ring_consume(m_pRingH264, NULL, &buff, 1);
            if(!ret) {
                Sleep(10);
                continue;
            }
            AV_BUFF tmp = {AV_TYPE::H264_NALU, buff.pData, buff.nLen};
            Recode2(tmp, buff.eType);
        }
        m_bFinish = true;
    }

    bool CRecoder::EncodeVideo(char *data,int size,int bIsKeyFrame)
    {
        /*
        m_decode->pkt.data = NULL;
        m_decode->pkt.size = 0;
        int len = av_parser_parse2(m_decode->parser, m_decode->c, 
        &(m_decode->pkt.data), &(m_decode->pkt.size), 
        (uint8_t*)buff.pData, buff.nLen, 
        AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

        if(m_decode->pkt.size==0){    // h264һ��֡û������
        //Log::error("not full");
        return 0;
        }
        */

        // 264����
        m_decode->pkt.data = (uint8_t*)data;
        m_decode->pkt.size = size;
        m_decode->pkt.pts = m_timestamp;
        m_decode->pkt.dts = m_timestamp;
        int ret = avcodec_send_packet(m_decode->c, &m_decode->pkt);
        if (ret != 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Decode Error %d:%s", ret, tmp);
            return false;
        }

        while(!ret){
            //Log::debug("receive frame");
            ret = avcodec_receive_frame(m_decode->c, m_decode->frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                //Log::error("decoding end");
                break;
            } else if (ret != 0) {
                Log::error("Error during decoding");
                break;
            }

            //Log::debug("src width:%d, height:%d", m_decode->frame->width, m_decode->frame->height);
            //Log::debug("context %d %d %lld %d %d", m_decode->c->width, m_decode->c->height, m_decode->c->bit_rate, m_decode->c->time_base.den, m_decode->c->time_base.num);
            //Log::debug("yuv pts: %lld  dts: %lld", m_decode->frame->pts, m_decode->frame->pkt_dts);

            //���ź��ͼƬ���½���h264����
            if(!m_codec->ok){
                Log::debug("first codec obj");
                m_codec->Start(m_decode);
            }
            if(!m_codec->ok){
                Log::error("codec is not ok");
                return false;
            }

            //int size = m_decode->frame->width * m_decode->frame->height;
            //fwrite(m_decode->frame->data[0], 1, size, m_decode->f);
            //fwrite(m_decode->frame->data[1], 1, size/4, m_decode->f);
            //fwrite(m_decode->frame->data[2], 1, size/4, m_decode->f);
            //fflush(m_decode->f);
            //break;

            //�Ѿ�ȡ��һ֡yuvͼƬ����������


            //Log::debug("begin scale");
            int h = sws_scale(m_codec->swsc, m_decode->frame->data, m_decode->frame->linesize, 0, m_decode->frame->height
                , m_codec->frame->data, m_codec->frame->linesize);
            m_codec->frame->pts = m_timestamp;
            m_codec->frame->pkt_dts = m_timestamp;

            //Log::debug("height:%d line0:%d line1:%d line2:%d", h, m_codec->frame->linesize[0], m_codec->frame->linesize[1], m_codec->frame->linesize[2]);
            //Log::debug("scale pts: %lld  dts: %lld", m_codec->frame->pts, m_codec->frame->pkt_dts);

            //int size = m_codec->frame->width * m_codec->frame->height;
            //fwrite(m_codec->frame->data[0], 1, size, m_codec->yuv);
            //fwrite(m_codec->frame->data[1], 1, size/4, m_codec->yuv);
            //fwrite(m_codec->frame->data[2], 1, size/4, m_codec->yuv);
            //fflush(m_codec->yuv);


            //Log::debug("begin encode");
            m_codec->frame->pts = m_timestamp;
            int cret = avcodec_send_frame(m_codec->c, m_codec->frame);

            if (cret == AVERROR(EAGAIN) || cret == AVERROR_EOF){
                Log::error("avcodec_send_frame end");
                break;
            } else if (cret < 0) {
                char tmp[1024]={0};
                av_strerror(cret, tmp, 1024);
                Log::error("avcodec_send_frame error: %s", tmp);
                break;
            }

            m_codec->pkt.data = NULL;
            m_codec->pkt.size = 0;
            cret = avcodec_receive_packet(m_codec->c, &m_codec->pkt);
            if (cret != 0) {
                char tmp[1024]={0};
                av_strerror(cret, tmp, 1024);
                Log::error("avcodec_receive_packet error: %s", tmp);
                break;
            }
            //Log::debug("recode finish size:%d, pts:%d, dts:%d", m_codec->pkt.size, m_codec->pkt.pts, m_codec->pkt.dts);

            //fwrite(m_codec->pkt.data, 1, m_codec->pkt.size, m_codec->f);
            //fflush(m_codec->f);
            AV_BUFF ESBUFF = {AV_TYPE::ES, (char*)m_codec->pkt.data, m_codec->pkt.size};
            m_codec->pEs->DeCode(ESBUFF);
        } //while

        return true;
    }

    bool CRecoder::EncodeKeyVideo()
    {
        if(m_pSPS->size() && m_pPPS->size() && m_pKeyFrame->size()) {
            int bufsize = m_pSPS->size() + m_pPPS->size() + m_pKeyFrame->size();
            char* buff = (char*)malloc(bufsize);
            int pos = 0;
            memcpy(buff+pos, m_pSPS->get(), m_pSPS->size());
            pos += m_pSPS->size();
            memcpy(buff+pos, m_pPPS->get(), m_pPPS->size());
            pos += m_pPPS->size();
            memcpy(buff+pos, m_pKeyFrame->get(), m_pKeyFrame->size());
            pos += m_pKeyFrame->size();

            //Log::debug("send key frame");
            EncodeVideo(buff, bufsize, 1);
            free(buff);

            //Log::debug("send sps");
            //MakeVideo(m_pSPS->get(),m_pSPS->size(),1);
            //Log::debug("send pps");
            //MakeVideo(m_pPPS->get(),m_pPPS->size(),1);
            //Log::debug("send key frame");
            //MakeVideo(m_pKeyFrame->get(),m_pKeyFrame->size(),1);

            m_pKeyFrame->clear();
            m_bGotSPS = false;
            m_bGotPPS = false;

            m_timestamp += m_tick_gap;
            m_bFirstKey = true;

            return true;
        }
        return false;
    }
}
#endif