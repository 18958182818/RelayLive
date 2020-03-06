
#include "uv.h"
#include "libwebsockets.h"
#include "netstream.h"
#include "live.h"
#include "rtp.h"
#include "worker.h"
#include "ipc.h"
#include "util.h"
#include "Log.h"
#include <list>
#include <sstream>

#ifdef USE_FFMPEG
extern "C"
{
#define snprintf  _snprintf
#define __STDC_FORMAT_MACROS
#include "libavdevice/avdevice.h"
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h"
#include "libavutil/timestamp.h"
#include "libavutil/opt.h"
}
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"postproc.lib")
#pragma comment(lib,"swresample.lib")
#pragma comment(lib,"swscale.lib")
#endif

namespace Server
{
    typedef struct _AV_BUFF_ {
        char       *pData;    //< ��������
        uint32_t    nLen;     //< ���ݳ���
    }AV_BUFF;

    static list<CLiveWorker*>  _listWorkers;
    static CriticalSection     _csWorkers;
	static uint32_t            _flvbufsize = 1024*16;
	static uint32_t            _psbufsize = 1024*32;
	FILE *_f = NULL, *_f2 = NULL, *_f3 = NULL;

    static void destroy_ring_node(void *_msg)
    {
        AV_BUFF *msg = (AV_BUFF*)_msg;
        free(msg->pData);
        msg->pData = NULL;
        msg->nLen = 0;
    }

    static string GetClientsInfo() {
        bool first = true;
        stringstream ss;
        for (auto c : _listWorkers) {
            if(!first) {
                first = false;
                ss << ",";
            }
            ss << "{\"DeviceID\":\"" << c->m_strCode 
                << "\",\"Connect\":\"";
            if(c->m_bWebSocket)
                ss << "websocket";
            else
                ss << "http";
            ss << "\",\"Media\":\"" << c->m_strType
                << "\",\"ClientIP\":\"" << c->m_strClientIP
                << "\",\"Channel\":\"" << c->m_nWidth << "*" << c->m_nHeight
                << "\"}";
        }
        return ss.str();
    }

    //////////////////////////////////////////////////////////////////////////

    // �����߳�
    static void real_play(void* arg){
        CLiveWorker* pLive = (CLiveWorker*)arg;
        pLive->Play();
    }

#ifdef USE_FFMPEG
    //��ȡ�������ݵķ���
    static int fill_iobuffer(void *opaque,uint8_t *buf, int bufsize){
        CLiveWorker *lw = (CLiveWorker*)opaque;
        int len = 0;
        while (len == 0) {
            len = lw->get_ps_data((char*)buf, bufsize);
        }
		//fwrite(buf, 1, len, _f2);
		//fflush(_f2);
        return len;
    }

    //������ݵķ���
    static int write_buffer(void *opaque, uint8_t *buf, int buf_size){
        CLiveWorker* pLive = (CLiveWorker*)opaque;
        pLive->push_flv_frame((char*)buf, buf_size);
        return buf_size;
    }
#endif

    CLiveWorker::CLiveWorker()
        : m_bWebSocket(false)
        , m_nWidth(0)
        , m_nHeight(0)
		, m_bConnect(true)
		, m_bParseKey(false)
		, m_pTmpBuff(NULL)
		, m_nTmpBuffSize(0)
		, m_nTmpBuffTotalSize(0)
		, m_nTmpBuffReaded(0)
    {
        m_pFlvRing  = create_ring_buff(sizeof(AV_BUFF), 100, destroy_ring_node);
        m_pPSRing   = create_ring_buff(sizeof(AV_BUFF), 1000, destroy_ring_node);
		//_f = fopen("D:\\code\\RelayLive_new\\out\\x64_Debug\\ps.dat", "wb");
		//_f2 = fopen("D:\\code\\RelayLive_new\\out\\x64_Debug\\ps2.dat", "wb");
		//_f3 = fopen("D:\\code\\RelayLive_new\\out\\x64_Debug\\ps3.dat", "wb");
    }

    CLiveWorker::~CLiveWorker()
    {
        destroy_ring_buff(m_pFlvRing);
        destroy_ring_buff(m_pPSRing);
        Log::debug("CLiveWorker release");
    }

#ifdef USE_FFMPEG
    bool CLiveWorker::Play()
    {
        IPC::PlayRequest req = IPC::RealPlay(m_strCode);
        if(req.ret != 0) {
            Log::error("play %s failed: %s", m_strCode.c_str(), req.info.c_str());
            return false;
        }

        void *playHandle = RtpDecode::Play(this, req.info, req.port);


        AVFormatContext *ifc = NULL;               //�����װ
        AVFormatContext *ofc = NULL;               //�����װ
        AVStream        *istream_video = NULL;     //������Ƶ����
        AVStream        *ostream_video = NULL;     //�����Ƶ����
        AVCodecContext  *decode_ctx = NULL;        //������Ƶ����
        AVCodecContext  *encode_ctx = NULL;        //�����Ƶ����
        AVFrame         *frame = NULL;             //ԭʼ��������֡
        AVFrame         *filt_frame = NULL;        //filter���ź��֡
        AVFilterGraph   *filter_graph = NULL;      //filter����
        AVFilterContext *buffersink_ctx = NULL;    //����filter
        AVFilterContext *buffersrc_ctx = NULL;     //���filter
        bool            recodec_video = false;     //��Ƶ�����Ƿ���Ҫ���½����
        bool            scale_video = false;       //��Ƶͼ���Ƿ���Ҫ����
        int             in_video_index = -1;
        int             out_video_index = 0;

        //��������
        ifc = avformat_alloc_context();
        unsigned char * iobuffer=(unsigned char *)av_malloc(_psbufsize);
        AVIOContext *avio = avio_alloc_context(iobuffer, _psbufsize, 0, this, fill_iobuffer, NULL, NULL);
        ifc->pb = avio;
		ifc->iformat = av_find_input_format("mpeg");

        int ret = avformat_open_input(&ifc, "nothing", NULL, NULL);
        if (ret != 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Could not open input file: %d(%s)", ret, tmp);
            goto end;
        }
		ifc->probesize = 51200;
		ifc->max_analyze_duration = 1*AV_TIME_BASE; //̽��ֻ������ʱ1s
        ret = avformat_find_stream_info(ifc, NULL);
        if (ret < 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Failed to retrieve input stream information %d(%s)", ret, tmp);
            goto end;
        }
        Log::debug("show input format info");
        av_dump_format(ifc, 0, "nothing", 0);

        AVCodec *dec;
        in_video_index = av_find_best_stream(ifc, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
        if (in_video_index < 0) {
            Log::error( "Cannot find a video stream in the input file");
            goto end;
        }
        istream_video = ifc->streams[in_video_index];

        if (istream_video->codecpar->codec_id != AV_CODEC_ID_H264)
            recodec_video = true;
        if ( (m_nWidth > 0  && istream_video->codecpar->width > m_nWidth)
            || (m_nHeight > 0 && istream_video->codecpar->height > m_nHeight)
        ) {
            recodec_video = true;
            scale_video = true;
        }

        if(recodec_video){
            //��Ҫ���±����
            decode_ctx = avcodec_alloc_context3(dec);
            avcodec_parameters_to_context(decode_ctx, istream_video->codecpar);
            decode_ctx->time_base.num = 1;
            decode_ctx->time_base.den = 25;

            /* init the video decoder */
            if ((ret = avcodec_open2(decode_ctx, dec, NULL)) < 0) {
                Log::error("Cannot open video decoder");
                return ret;
            }

            frame = av_frame_alloc();
            if (!frame) {
                Log::error("Could not allocate frame");
                goto end;
            }
        }
        if(scale_video) {
            filt_frame = av_frame_alloc();
            if (!filt_frame) {
                Log::error("Could not allocate filter_frame");
                goto end;
            }
        }


        //�������
        ret = avformat_alloc_output_context2(&ofc, NULL, m_strType.c_str(), NULL);
        if (!ofc) {
            Log::error("Could not create output context");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        unsigned char* outbuffer=(unsigned char*)av_malloc(_flvbufsize);
        AVIOContext *avio_out =avio_alloc_context(outbuffer, _flvbufsize,1,this,NULL,write_buffer,NULL);  
        ofc->pb = avio_out; 
        ofc->flags = AVFMT_FLAG_CUSTOM_IO;

        AVOutputFormat *ofmt = ofc->oformat;
        ofmt->flags |= AVFMT_NOFILE;

        //������е���Ƶ��
        ostream_video = avformat_new_stream(ofc, NULL);
        if (!ostream_video) {
            Log::error("Failed allocating output stream");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        ret = avcodec_parameters_copy(ostream_video->codecpar, istream_video->codecpar);
        if (ret < 0) {
            Log::error("Failed to copy codec parameters");
            goto end;
        }
        ostream_video->codecpar->codec_id = AV_CODEC_ID_H264;
        if(scale_video) {
            ostream_video->codecpar->width = m_nWidth;
            ostream_video->codecpar->height = m_nHeight;
        }

        if(recodec_video) {
            // ��Ƶ���±���Ϊh264
            AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
            encode_ctx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(encode_ctx, ostream_video->codecpar);

            encode_ctx->bit_rate = 400000;
            encode_ctx->width = ostream_video->codecpar->width;
            encode_ctx->height = ostream_video->codecpar->height;
            encode_ctx->time_base.num = 1;
            encode_ctx->time_base.den = 25;
            encode_ctx->framerate.num = 25;
            encode_ctx->framerate.den = 1;
            encode_ctx->gop_size = 15;
            encode_ctx->has_b_frames = 0;
            encode_ctx->max_b_frames = 0;
            encode_ctx->qmin = 34;
            encode_ctx->qmax = 50;

            av_opt_set(encode_ctx->priv_data, "preset",  "slow", 0);
            av_opt_set(encode_ctx->priv_data, "tune",    "zerolatency", 0);
            av_opt_set(encode_ctx->priv_data, "profile", "main", 0);

            ret = avcodec_open2(encode_ctx, codec, NULL);
            if (ret < 0) {
                Log::error("can not open encoder");
                goto end;
            }
        }

        Log::debug("show output format info");
        av_dump_format(ofc, 0, NULL, 1);

        ret = avformat_write_header(ofc, NULL);
        if (ret < 0) {
            char tmp[1024]={0};
            av_strerror(ret, tmp, 1024);
            Log::error("Error avformat_write_header %d:%s", ret, tmp);
            goto end;
        }

        if(scale_video) {
            //��ʼ��������Ƶ��filter
            filter_graph = avfilter_graph_alloc();

            //Դfilter
            char args[512]={0};
            sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                decode_ctx->width, decode_ctx->height, decode_ctx->pix_fmt,
                istream_video->time_base.num, istream_video->time_base.den,
                decode_ctx->sample_aspect_ratio.num, decode_ctx->sample_aspect_ratio.den);
            const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
            ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
            if (ret < 0) {
                Log::error("Cannot create buffer source");
                goto end;
            }

            //sink filter
            const AVFilter *buffersink = avfilter_get_by_name("buffersink");
            AVBufferSinkParams *buffersink_params = av_buffersink_params_alloc();
            enum AVPixelFormat pix_fmts[] = { encode_ctx->pix_fmt, AV_PIX_FMT_NONE };
            buffersink_params->pixel_fmts = pix_fmts;
            ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, buffersink_params, filter_graph);
            av_free(buffersink_params);
            if (ret < 0) {
                Log::error("Cannot create buffer sink");
                goto end;
            }

            //Endpoints for the filter graph
            AVFilterInOut *outputs = avfilter_inout_alloc();
            outputs->name       = av_strdup("in");
            outputs->filter_ctx = buffersrc_ctx;
            outputs->pad_idx    = 0;
            outputs->next       = NULL;

            AVFilterInOut *inputs  = avfilter_inout_alloc();
            inputs->name       = av_strdup("out");
            inputs->filter_ctx = buffersink_ctx;
            inputs->pad_idx    = 0;
            inputs->next       = NULL;

            char filter_descr[20]={0};
            sprintf(filter_descr, "scale=%d:%d", m_nWidth, m_nHeight);
            ret = avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL);
            if (ret < 0){
                Log::error("Cannot filter graph parse ptr");
                goto end;
            }

            ret = avfilter_graph_config(filter_graph, NULL);
            if (ret < 0){
                Log::error("Cannot filter graph config");
                goto end;
            }
        }

        while (m_bConnect) {
            AVPacket pkt;
            av_init_packet(&pkt);

            ret = av_read_frame(ifc, &pkt);
            if (ret < 0)
                break;

            if (pkt.stream_index == in_video_index) {
                // ��Ƶ����
                if(recodec_video) {
                    // ����ԭʼ����
                    ret = avcodec_send_packet(decode_ctx, &pkt);
                    if (ret < 0) {
                        Log::error("decoding video stream failed");
                        goto end;
                    }

                    while (true) {
                        ret = avcodec_receive_frame(decode_ctx, frame);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        } else if (ret < 0) {
                            Log::error("Error while receiving a frame from the decoder\n");
                            break;
                        }

                        frame->pts = frame->best_effort_timestamp;

                        if(scale_video) {
                            // ��Ҫͨ��filter����֡
                            ret = av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
                            if (ret < 0) {
                                Log::error("Error while feeding the filtergraph");
                                goto end;
                            }
                            while (true) {
                                ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                                if (ret < 0) {
                                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                                        ret = 0;
                                    }
                                    break;
                                }

                                //���ź����Ƶ֡���벢д�����
                                AVPacket enc_pkt;
                                av_init_packet(&enc_pkt);
                                ret = avcodec_send_frame(encode_ctx, filt_frame);
                                if (ret < 0)
                                    break;

                                while (true) {
                                    ret = avcodec_receive_packet(encode_ctx, &enc_pkt);
                                    if (ret < 0)
                                        break;


                                    enc_pkt.stream_index = out_video_index;
									enc_pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
									enc_pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
									enc_pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
									enc_pkt.pos = -1;
                                    ret = av_interleaved_write_frame(ofc, &enc_pkt);
									if (ret < 0) {
										char tmp[1024]={0};
										av_strerror(ret, tmp, 1024);
										Log::error("video error muxing packet %d:%s", ret, tmp);
										//break;
									}

                                    av_packet_unref(&enc_pkt);
                                }
                                av_frame_unref(filt_frame);
                            }
                        } else {
                            // ����Ҫ���ţ�ֱ���ر��벢д�����
                            AVPacket enc_pkt;
                            av_init_packet(&enc_pkt);
                            ret = avcodec_send_frame(encode_ctx, frame);
                            if (ret < 0)
                                break;

                            while (true) {
                                ret = avcodec_receive_packet(encode_ctx, &enc_pkt);
                                if (ret < 0)
                                    break;

                                enc_pkt.stream_index = out_video_index;
								enc_pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
								enc_pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
								enc_pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
								enc_pkt.pos = -1;
                                ret = av_interleaved_write_frame(ofc, &enc_pkt);
								if (ret < 0) {
									char tmp[1024]={0};
									av_strerror(ret, tmp, 1024);
									Log::error("video error muxing packet %d:%s", ret, tmp);
									//break;
								}

                                av_packet_unref(&enc_pkt);
                            }
                        }

                        av_frame_unref(frame);
                    }
                } else {
                    // ��ԭʼ����д�����
                    pkt.stream_index = out_video_index;
					pkt.pts = av_rescale_q_rnd(pkt.pts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
					pkt.dts = av_rescale_q_rnd(pkt.dts, istream_video->time_base, ostream_video->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
					pkt.duration = av_rescale_q(pkt.duration, istream_video->time_base, ostream_video->time_base);
					pkt.pos = -1;
                    ret = av_interleaved_write_frame(ofc, &pkt);
                    if (ret < 0) {
                        char tmp[1024]={0};
                        av_strerror(ret, tmp, 1024);
                        Log::error("video error muxing packet %d:%s", ret, tmp);
                        //break;
                    }
                }
            } //else if (pkt.stream_index == in_audio_index) {
            //    //Log::debug("audio dts %d pts %d", pkt.dts, pkt.pts);
            //    pkt.stream_index = out_audio_index;
            //    out_stream = ofc->streams[pkt.stream_index];
            //    /* copy packet */
            //    pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
            //    pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF/*|AV_ROUND_PASS_MINMAX*/);
            //    pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            //    pkt.pos = -1;
            //    //Log::debug("audio2 dts %d pts %d", pkt.dts, pkt.pts);
            //
            //    int wret = av_interleaved_write_frame(ofc, &pkt);
            //    if (wret < 0) {
            //        char tmp[1024]={0};
            //        av_strerror(ret, tmp, 1024);
            //        Log::error("audio error muxing packet %d:%s \n", ret, tmp);
            //        //break;
            //    }
            //}
            av_packet_unref(&pkt);
        }

        av_write_trailer(ofc);
end:
        /** ����filter */
        av_frame_free(&frame);
        av_frame_free(&filt_frame);
        avfilter_graph_free(&filter_graph);

        /** �رս��� */
        avcodec_close(decode_ctx);
        avcodec_close(encode_ctx);

        /* �ر�������� */
        avformat_close_input(&ifc);
        if (ofc && !(ofmt->flags & AVFMT_NOFILE))
            avio_closep(&ofc->pb);
        avformat_free_context(ofc);

        /** ������ */
        if (ret < 0 /*&& ret != AVERROR_EOF*/) {
            char tmp[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(tmp,AV_ERROR_MAX_STRING_SIZE,ret);
            Log::error("Error occurred: %s", tmp);
        }

        //�ر�rtp
        RtpDecode::Stop(playHandle);

        if(m_bConnect) {
            Log::debug("video source is dropped, need close the client");
            //lws_set_timeout(m_pPss->wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        }
        while (m_bConnect){
            sleep(10);
        }
        Log::debug("client stop, delete live worker");
        delete this;
        return ret>=0;
    }
#else
bool CLiveWorker::Play() {
}
#endif

    void CLiveWorker::push_ps_data(char* pBuff, int nLen)
    {
		//static int num = 0;
		//char path[MAX_PATH] = {0};
		//sprintf(path, "D:\\code\\RelayLive_new\\out\\x64_Debug\\ps\\%04d.ps", num++);
		//FILE *f = fopen(path, "wb");
		//fwrite(pBuff, 1, nLen, f);
		//fclose(f);
		//fwrite(pBuff, 1, nLen, _f);
		//fflush(_f);
		//return;
        //�ڴ����ݱ�����ring-buff
		int n = (int)ring_get_count_free_elements(m_pPSRing);
		if (!n) {
			Log::error("to many ps data can't catch");
			return;
		}

		//fwrite(pBuff, 1, nLen, _f3);
		//fflush(_f3);

		// �����ݱ�����ring buff
		char* pSaveBuff = (char*)malloc(nLen);
		memcpy(pSaveBuff, pBuff, nLen);
		AV_BUFF newTag = {pSaveBuff, nLen};
		if (!ring_insert(m_pPSRing, &newTag, 1)) {
			destroy_ring_node(&newTag);
			Log::error("dropping!");
			return;
		}
		//Log::debug("push ps %x, %d, %d", newTag.pData, newTag.nLen, ring_get_count_free_elements(m_pPSRing));
    }

    int CLiveWorker::get_ps_data(char* pBuff, int &nLen)
    {
		if(m_nTmpBuffReaded >= m_nTmpBuffSize) {
			AV_BUFF* tag = (AV_BUFF*)ring_get_element(m_pPSRing, NULL);
			if(!tag) 
				return 0;

			if(!m_pTmpBuff) {
				m_pTmpBuff = (char*)malloc(tag->nLen);
				m_nTmpBuffTotalSize = tag->nLen;
			} else if(m_nTmpBuffTotalSize < tag->nLen) {
				free(m_pTmpBuff);
				m_pTmpBuff = (char*)malloc(tag->nLen);
				m_nTmpBuffTotalSize = tag->nLen;
			}
			memcpy(m_pTmpBuff, tag->pData, tag->nLen);
			m_nTmpBuffSize = tag->nLen;
			m_nTmpBuffReaded = 0;
			simple_ring_cosume(m_pPSRing);

			//Log::debug("get ps %x, %d, %d", tag->pData, tag->nLen, ring_get_count_free_elements(m_pPSRing));

			//if(!m_bParseKey) {
			//	if(is_key(tag->pData, tag->nLen)){
			//		m_bParseKey = true;
			//	} else {
			//		Log::warning("no key frame before this");
			//		simple_ring_cosume(m_pPSRing);
			//		return 0;
			//	}
			//}
			//fwrite(tag->pData, 1, tag->nLen, _f);
			//fflush(_f);
		}

		int len = m_nTmpBuffSize - m_nTmpBuffReaded;
		if(len > nLen)
			len = nLen;
		memcpy(pBuff, m_pTmpBuff+m_nTmpBuffReaded, len);
		m_nTmpBuffReaded += len;

        return len;
    }

    void CLiveWorker::push_flv_frame(char* pBuff, int nLen)
    {
        if(!m_bConnect) {
            Log::error("client has already leave");
            return;
        }

        //�ڴ����ݱ�����ring-buff
        int n = (int)ring_get_count_free_elements(m_pFlvRing);
        if (!n && m_pPss) {
            //lws_set_timeout(m_pPss->wsi, PENDING_TIMEOUT_LAGGING, LWS_TO_KILL_ASYNC);
			lws_callback_on_writable(m_pPss->wsi);
            Log::error("to many data can't send");
            return;
        }
       printf("flv_free(%d) ", n);

        // �����ݱ�����ring buff
        char* pSaveBuff = (char*)malloc(nLen + LWS_PRE);
        memcpy(pSaveBuff + LWS_PRE, pBuff, nLen);
        AV_BUFF newTag = {pSaveBuff, nLen};
        if (!ring_insert(m_pFlvRing, &newTag, 1)) {
            destroy_ring_node(&newTag);
            Log::error("dropping!");
            return;
        }

        //��ͻ��˷�������
		if(m_pPss)
			lws_callback_on_writable(m_pPss->wsi);
    }

    int CLiveWorker::get_flv_frame(char **buff)
    {
		AV_BUFF *tmp = (AV_BUFF*)simple_ring_get_element(m_pFlvRing);
		if(tmp == NULL)
			return 0;
		*buff = tmp->pData + LWS_PRE;
		return tmp->nLen;
    }

	void CLiveWorker::next_flv_frame() {
		simple_ring_cosume(m_pFlvRing);
		if (ring_get_count_waiting_elements(m_pFlvRing, NULL) && m_pPss)
			lws_callback_on_writable(m_pPss->wsi);
	}

	void CLiveWorker::close()
	{
        _csWorkers.lock();
        _listWorkers.remove(this);
        IPC::SendClients(GetClientsInfo());
        _csWorkers.unlock();
		m_bConnect = false;
		m_pPss = NULL;
	}

	bool CLiveWorker::is_key(char* pBuff, int nLen) {
		if(nLen <= 4) return false;
		uint8_t *data = (uint8_t*)pBuff;
		for(int i=0; i <nLen-4; ++i) {
			if(data[i] == 0 && data[i+1] == 0 && data[i+2] == 1 && data[i+3] == 0x67)
				return true;
		}
		return false;
	}
    
    CLiveWorker* CreatLiveWorker(string strCode, string strType, string strHw, bool isWs, pss_live *pss, string clientIP) {
        CLiveWorker *worker = new CLiveWorker();
        worker->m_strCode = strCode;
        worker->m_strType = strType;
        if(!strHw.empty() && strHw.find('*') != string::npos)
            sscanf(strHw.c_str(), "%d*%d", &worker->m_nWidth, &worker->m_nHeight);
        worker->m_bWebSocket = isWs;
        worker->m_pPss = pss;
		worker->m_strClientIP = clientIP;
        if(strType == "flv")
            worker->m_strMIME = "video/x-flv";
        else if(strType == "h264")
            worker->m_strMIME = "video/h264";
        else if(strType == "mp4")
            worker->m_strMIME = "video/mp4";
        pss->pWorker = worker;

        _csWorkers.lock();
        _listWorkers.push_back(worker);
        IPC::SendClients(GetClientsInfo());
        _csWorkers.unlock();

        uv_thread_t tid;
        uv_thread_create(&tid, real_play, (void*)worker);
        Log::debug("RealPlay ok: %s",strCode.c_str());
        return worker;
    }

#ifdef USE_FFMPEG
    static void ffmpeg_log_cb(void* ptr, int level, const char* fmt, va_list vl){
        char text[256];              //��־����
        memset(text,0,256);
        vsprintf_s(text, 256, fmt, vl);

        if(level <= AV_LOG_ERROR){
            Log::error(text);
        } else if(level == AV_LOG_WARNING) {
            Log::warning(text);
        } else {
            Log::debug(text);
        }
    }

    void InitFFmpeg(){
        //av_log_set_callback(ffmpeg_log_cb);
    }
#endif
}