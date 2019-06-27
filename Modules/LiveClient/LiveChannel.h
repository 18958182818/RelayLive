/*!
 * \file LiveChannel.h
 * \date 2019/06/21 16:45
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief 
 *
 * TODO: ��ͨ����Ƶ����
 *
 * \note
*/

#pragma once
#include "LiveClient.h"
#include "avtypes.h"
#include "ts.h"
#include "flv.h"
#include "mp4.h"
#include "h264.h"
#include "uv.h"
#include "Recode.h"

namespace LiveClient
{
    class CLiveChannel
    {
    public:
        /** ԭʼͨ������ */
        CLiveChannel();
        /** ����ͨ������ */
        CLiveChannel(int channel, uint32_t w, uint32_t h);
        ~CLiveChannel();

#ifdef USE_FFMPEG
        /**
         * h264������,�������ݲ���
         */
        void SetDecoder(IDecoder *decoder);
#endif

        /**
         * ��ͨ����Ӳ��ſͻ���
         * @param h �ͻ���ʵ��
         * @param t ��������
         */
        bool AddHandle(ILiveHandle* h, HandleType t);

        /**
         * �Ƴ�һ�����ſͻ���
         * @param h �ͻ���ʵ��
         * return true:���пͻ��˶����Ƴ� false:��Ȼ���ڿͻ���
         */
        bool RemoveHandle(ILiveHandle* h);

        /** ͨ�����Ƿ���ڲ��ſͻ��� */
        bool Empty();

        /**
         * ����Դ���ݣ�ԭʼͨ������ѹ���õ�����������ͨ�����ս�����yuv����
         */
        void ReceiveStream(AV_BUFF buff);

        /** ��ȡ�ļ�ͷ */
        AV_BUFF GetHeader(HandleType t);

        /** ��ȡ�ͻ�����Ϣ */
        vector<ClientInfo> GetClientInfo();

        /** H264��sps�����ص� */
        void set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps);

        /**
        * ��Դ��������Ƶ���ݣ����߳����� 
        * ���·�����rtp�������ڵ�loop�̵߳���
        * �������������������졢��������http���ڵ�loop�̵߳���
        */
        void FlvCb (AV_BUFF buff);
        void H264Cb(AV_BUFF buff);
        void TsCb  (AV_BUFF buff);
        void Mp4Cb (AV_BUFF buff);
        void RtpCb (AV_BUFF buff);
        void RtcpCb(AV_BUFF buff);
        void stop();

        bool m_bFlv;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
        AV_BUFF               m_stFlvHead;    //flvͷ�����ݴ洢��CFlv����
        AV_BUFF               m_stMp4Head;    //mp4ͷ�����ݴ洢��CMP4����

    private:
        void Init();
        void push_h264_stream(AV_BUFF buff);

        vector<ILiveHandle*>     m_vecLiveFlv;  // ����ʵ�� 
        CriticalSection          m_csFlv;
        vector<ILiveHandle*>     m_vecLiveMp4;  // ����ʵ�� 
        CriticalSection          m_csMp4;
        vector<ILiveHandle*>     m_vecLiveH264; // ����ʵ�� 
        CriticalSection          m_csH264;
        vector<ILiveHandle*>     m_vecLiveTs;   // ����ʵ�� 
        CriticalSection          m_csTs;
        vector<ILiveHandleRtp*>  m_vecLiveRtp;  // ����ʵ�� 
        CriticalSection          m_csRtp;

        int                      m_nChannel;    // ͨ����
        CH264                   *m_pH264;       // H264�����
        CTS                     *m_pTs;         // TS�����
        CFlv                    *m_pFlv;        // FLV�����
        CMP4                    *m_pMp4;        // MP4�����

#ifdef USE_FFMPEG
        IEncoder                *m_pEncoder;    // YUV����Ϊh264
#endif

        uint32_t                 m_nWidth;
        uint32_t                 m_nHeight;
    };

}