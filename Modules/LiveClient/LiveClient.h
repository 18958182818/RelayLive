/**
* ���ļ���libLiveģ��Ψһ���⵼����ͷ�ļ�
*/
#pragma once

#ifdef LIVECLIENT_EXPORTS
#define LIVECLIENT_API __declspec(dllexport)
#else
#define LIVECLIENT_API
#endif

enum HandleType
{
    unknown_handle,
    flv_handle,
    fmp4_handle,
    h264_handle,
    ts_handle,
    rtp_handle,
};

namespace LiveClient
{
    /**
    * ���ݻص�����ӿ�
    * ���ϲ�ʵ��һ���̳иýӿڵ���������RTP�����������
    */
    struct ILiveHandle
    {
        /**
         * ��Ƶ���ݷ��ͻص�
         */
        virtual void push_video_stream(char*, int) = 0;
        /**
        * rtp���ն˽�����Ŀǰ��ֻ֪�н��ճ�ʱ����
        */
        virtual void stop() = 0;
 
    };

    struct ILiveHandleRtp : public ILiveHandle {
        virtual void push_rtcp_stream(char*, int) = 0;
    };

    struct ILiveWorker
    {
        /**
        * ��liveworker�����һ���ص������
        * liveworker��Ӧ���livehandle
        * @param pWorker Liveworker�������rtp���ݲ������ɸ��ָ�ʽ
        * @param h ��Ҫ��Ƶ��������ִ�и��ִ�������Ϊ�����͵��ͻ���
        */
        virtual bool AddHandle(ILiveHandle* h, HandleType t) = 0;

        /**
        * ��liveworker���Ƴ�һ��livehandle��
        * ��liveworker�е�handleȫ���Ƴ�ʱ��liveworker������ɱ
        */
        virtual bool RemoveHandle(ILiveHandle* h) = 0;
    };

    /**
    * liveclient��ʼ��
    */
    void LIVECLIENT_API Init();

    /**
    * ��ȡ�ͻ�������json�ַ���
    */
    string LIVECLIENT_API GetClientsInfo();

    /**
    * ��ȡ�豸��Ϣ�������첽�������˴�ֻ��������
    * @param h ��������ľ��
    */
    void LIVECLIENT_API GetDevList();

    /**
     * 
     */
    void LIVECLIENT_API QueryDirtionary();

    /**
     * ���ûص�����ȡ��Ϣ��ɺ�,֪ͨ������
     */
    typedef void(*LIVECLIENT_CB)(string, string);
    void LIVECLIENT_API SetCallBack(LIVECLIENT_CB cb);

    /**
    * ��ȡһ���豸��ֱ����������Ѵ���ֱ��ʹ�ã����������򴴽�һ���µ�
    * @param strCode �豸�Ĺ������
    */
    ILiveWorker* LIVECLIENT_API GetWorker(string strCode);
}