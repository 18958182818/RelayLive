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

namespace LiveClient
{
    class CLiveChannel
    {
    public:
        CLiveChannel(int channel = 0);
        ~CLiveChannel();

        void SetParam(uint32_t w, uint32_t h){
            m_nWidth = w;
            m_nHeight = h;
        }

        bool AddHandle(ILiveHandle* h, HandleType t);
        bool RemoveHandle(ILiveHandle* h);
        bool Empty();

        AV_BUFF GetHeader(HandleType t);

        /** ��ȡ�ͻ�����Ϣ */
        string GetClientInfo();

        /** H264��sps�����ص� */
        void set_h264_param(uint32_t nWidth, uint32_t nHeight, double fFps);
        /**
        * ��Դ��������Ƶ���ݣ����߳����� 
        * ���·�����rtp�������ڵ�loop�̵߳���
        * �������������������졢��������http���ڵ�loop�̵߳���
        */
        void push_flv_stream (AV_BUFF buff);
        void push_h264_stream(AV_BUFF buff);
        void push_ts_stream  (AV_BUFF buff);
        void push_fmp4_stream(AV_BUFF buff);
        void push_rtp_stream (AV_BUFF buff);
        void push_rtcp_stream(AV_BUFF buff);
        void stop();

        bool m_bFlv;
        bool m_bMp4;
        bool m_bH264;
        bool m_bTs;
        bool m_bRtp;
        AV_BUFF               m_stFlvHead;    //flvͷ�����ݴ洢��CFlv����
        AV_BUFF               m_stMp4Head;    //mp4ͷ�����ݴ洢��CMP4����

    private:
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
        CH264                   *m_pH264;            // H264�����
        CTS                     *m_pTs;              // TS�����
        CFlv                    *m_pFlv;             // FLV�����
        CMP4                    *m_pMp4;             // MP4�����

        uint32_t                 m_nWidth;
        uint32_t                 m_nHeight;
    };

}