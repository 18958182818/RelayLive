/**
 * ���ļ���rtspģ��Ψһ���⵼����ͷ�ļ�
 */
#pragma once

#ifdef RTSP_EXPORTS
#define RTSP_API __declspec(dllexport)
#else
#define RTSP_API
#endif

enum NalType;
enum flv_tag_type;
enum MP4_FRAG_TYPE;

/**
 * ���ݻص�����ӿ�
 * ���ϲ�ʵ��һ���̳иýӿڵ���������RTP�����������
 */
struct IlibLiveCb
{
    /**
     * FLV���ݴ���ӿ�
     */
    virtual void push_flv_frame(int tag_type, char* frames, int frames_size) = 0;

    /**
     * TS���ݴ���ӿ�(HLS)
     */
    virtual void push_ts_stream(char* pBuff, int nBuffSize) = 0;

    /**
     * h264��������ӿ�
     */
    virtual void push_h264_stream(NalType eType, char* pBuff, int nBuffSize) = 0;

    /**
     * mp4���ݴ���ӿ�
     */
    virtual void push_mp4_stream(MP4_FRAG_TYPE eType, char* pBuff, int nBuffSize) = 0;

    /**
     * rtsp���ն˽���
     */
    virtual void stop() = 0;

    bool        m_bFlv;
    bool        m_bTs;
    bool        m_bH264;
    IlibLiveCb():m_bFlv(false),m_bTs(false),m_bH264(false){}
};

struct RTSP_API IlibLive
{
    virtual ~IlibLive(){}

    /**
     * ����һ��ӵ�б��ӿڹ��ܵ�ʵ��
     * return ʵ����ָ�룬ע��Ҫ���������ͷ�
     */
    static IlibLive* CreateObj();

    /**
     * ����flvͷ
     */
    static bool MakeFlvHeader(char** ppBuff, int* pLen);

    /**
     * ���ñ��ؼ�����IP�Ͷ˿�
     * @param[in] strIP ����IP
     * @param[in] nPort ������UDP�˿�
     */
    virtual void SetLocalAddr(string strIP, int nPort) = 0;

    /**
     * ���û���֡����
     * @param[in] nPacketNum ֡��������,��ֵԽ���ӳ�Խ�󣬵���Ӧ�Ը��������״��
     */
    virtual void SetCatchPacketNum(int nPacketNum) = 0;

    /** 
     * ����UDP�˿ڼ��� 
     */
    virtual void StartListen() = 0;

    /**
     * ���ûص��������
     * @param[in] pHandle �ص��������ָ��
     */
    virtual void SetCallback(IlibLiveCb* pHandle) = 0;
};
