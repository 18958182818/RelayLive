/*!
 * \file RtspSocket.h
 * \date 2019/06/10 15:51
 *
 * \author wlla
 * Contact: user@company.com
 *
 * \brief socket�շ�����
 *
 * TODO: rtsp����˼���socket������socket���Լ�rtp��rtcp��udp socket��ش���
 *
 * \note
*/
#pragma once

#include "uv.h"
#include "rtsp.h"
#include "RtspSession.h"

namespace RtspServer
{
    class CRtspServer;
    class CRtspSocket;

    typedef enum _RTSP_REASON_
    {
        RTSP_REASON_ERROR = 0,
        // rtsp���󷽷�
        RTSP_REASON_OPTIONS,
        RTSP_REASON_DESCRIBE,
        RTSP_REASON_SETUP,
        RTSP_REASON_PLAY,
        RTSP_REASON_PAUSE,
        RTSP_REASON_TEARDOWN,
        // �����¼�
        RTSP_REASON_CLOSE,
        RTSP_REASON_WRITE,
        RTSP_REASON_RTP_WRITE,
        RTSP_REASON_RTCP_WRITE
    }RTSP_REASON;

    typedef enum _response_code_
    {
        Code_100_Continue = 0,
        Code_110_ConnectTimeout,
        Code_200_OK,
        Code_201_Created,
        Code_250_LowOnStorageSpace,
        Code_300_MultipleChoices,
        Code_301_MovedPermanently,
        Code_302_MovedTemporarily,
        Code_303_SeeOther,
        Code_304_NotModified,
        Code_305_UseProxy,
        Code_350_GoingAway,
        Code_351_LoadBalancing,
        Code_400_BadRequest,
        Code_401_Unauthorized,
        Code_402_PaymentRequired,
        Code_403_Forbidden,
        Code_404_NotFound,
        Code_405_MethodNotAllowed,
        Code_406_NotAcceptable,
        Code_407_ProxyAuthenticationRequired,
        Code_408_RequestTimeOut,
        Code_410_Gone,
        Code_411_LengthRequired,
        Code_412_PreconditionFailed,
        Code_413_RequestEntityTooLarge,
        Code_414_RequestUriTooLarge,
        Code_415_UnsupportedMediaType,
        Code_451_ParameterNotUnderstood,
        Code_452_Reserved,
        Code_453_NotEnoughBandwidth,
        Code_454_SessionNotFound,
        Code_455_MethodNotValidInThisState,
        Code_456_HeaderFieldNotValidForResource,
        Code_457_InvalidRange,
        Code_458_ParameterIsReadOnly,
        Code_459_AggregateOperationNotAllowed,
        Code_460_OnlyAggregateOperationAllowed,
        Code_461_UnsupportedTransport,
        Code_462_DestinationUnreachable,
        Code_500_InternalServerError,
        Code_501_NotImplemented,
        Code_502_BadGateway,
        Code_503_ServiceUnavailable,
        Code_504_GatewayTimeOut,
        Code_505_RtspVersionNotSupported,
        Code_551_OptionNotSupported
    }response_code;

    /** rtsp��������������� */
    //typedef struct _rtsp_ruquest_
    //{
    //    rtsp_method     method;
    //    string          uri;
    //    bool            parse_status;
    //    response_code   code;
    //    uint64_t        CSeq;
    //    uint32_t        rtp_port;
    //    uint32_t        rtcp_port;
    //    map<string,string> headers;
    //}rtsp_ruquest;

    /** rtspӦ������ */
    typedef struct _rtsp_response_
    {
        response_code   code;
        uint32_t        CSeq;
        string          body;
        unordered_map<string,string> headers;
    }rtsp_response;

    /** �첽�¼� */
    typedef struct _rtsp_event_
    {
        RTSP_REASON  resaon;
    }rtsp_event;

    /** ���������� */
    typedef int(*live_rtsp_cb)(CRtspSocket *client, RTSP_REASON reason, void *user);
    struct rtsp_options
    {
        string ip;         //����IP
        int port;          //�����˿�
        int rtp_port;      //rtp�����󶨵���ʼ�˿�
        int rtp_port_num;  //rtp�˿���
        int user_len;      //�û���Ϣ�ṹ�Ĵ�С
        live_rtsp_cb cb;   //�û��Զ���ص�������
    };

    /**
     * �ͻ������ӵ�socket
     */
    class CRtspSocket
    {
    public:
        CRtspSocket();
        ~CRtspSocket();

        int Init(uv_loop_t* uv);
        int Recv();
        void parse(char* buff, int len);
        int answer(rtsp_ruquest_t *req);
        void SetRemotePort(int rtp, int rtcp);

        rtsp           *m_pRtspParse;   //rtsp���Ľ�������
        uv_tcp_t        m_rtsp;         //rtsp���Ӿ��
        //uv_udp_t        m_rtp;          //rtp���;��
        //uv_udp_t        m_rtcp;         //rtcp���;��
        uv_async_t      m_async;        //�߳̾��
        uv_loop_t*      m_ploop;
        list<rtsp_event> m_asyncList;  //�첽�¼��б�
        uv_mutex_t      m_asyncMutex;
        CRtspServer*    m_server;      //�������
        //CRtspWorker*    m_pWorker;     //ҵ�����
        CRtspSession   *m_pSession;
        CRtspSessionMgr *m_pSessMgr;
        string          m_strDevCode;  //�豸����
        string          m_strRtpIP;    //�ͻ���IP
        int             m_nRtpPort;    //�ͻ��˽���rtp���ݵĶ˿�
        int             m_nRtcpPort;   //�ͻ��˽���rtcp���ݵĶ˿�
		string          m_strLocalIP;  //����IP
        int             m_nLocalPort;  //���ط���rtp�Ķ˿�
        void*           m_user;        //�û�����
        struct sockaddr_in m_addrRtp;

        rtsp_ruquest_t* m_Request;
        rtsp_response*  m_Response;
    };

    /**
     * ����˼���socket
     */
    class CRtspServer
    {
    public:
        CRtspServer(rtsp_options options);
        ~CRtspServer(void);

        int Init(uv_loop_t* uv);

        uv_tcp_t        m_tcp;
        uv_loop_t*      m_ploop;

        rtsp_options    m_options;

        int GetRtpPort();
        void GiveBackRtpPort(int nPort);
        vector<int>     m_vecRtpPort;     //< RTP���ö˿ڣ�ʹ��ʱ����ȡ����ʹ�ý������·���
        CriticalSection m_csRTP;          //< RTP�˿���
    };


    

    /** �̰߳�ȫ�ķ�����֪ͨrtsploopִ��һ��RTP_WRITE�ص� */
    extern int rtp_callback_on_writable(CRtspSocket *client);

    /** �ڻص������в���ʹ�ã�����rtp���� */
    extern int rtp_write(CRtspSocket *client, char* buff, int len);

}