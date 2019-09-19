#include "stdafx.h"
#include "SipRegister.h"
#include "SipMgr.h"


CSipRegister::CSipRegister(eXosip_t* pSip)
    : m_pExContext(pSip)
    , m_strNonce("9bd055")
    , m_strAlgorithm("MD5")
    , m_bAuthorization(true)
{
}


CSipRegister::~CSipRegister(void)
{
}

void CSipRegister::SetAuthorization(bool bAuth)
{
    m_bAuthorization = bAuth;
}

void CSipRegister::OnRegister(eXosip_event_t *osipEvent)
{
    sipRegisterInfo regInfo;
    parserRegisterInfo(osipEvent->request, osipEvent->tid, regInfo);
    //��ӡ����
    printRegisterPkt(regInfo);
    //����Ӧ����
    sendRegisterAnswer(regInfo);

}

void CSipRegister::parserRegisterInfo(osip_message_t* request, int iReqId, sipRegisterInfo &regInfo)
{
    std::stringstream stream;

    if (nullptr == request)
    {
        Log::error("CSipRegister::parserRegisterInfo nullptr == request");
        return;
    }
    Log::debug("CSipRegister::parserRegisterInfo from user:%s,host:%s,port:%s; to user:%s,host:%s,port:%s"
        ,request->from->url->username,request->from->url->host,request->from->url->port
        ,request->to->url->username,request->to->url->host,request->to->url->port);

    regInfo.baseInfo.method = request->sip_method;
    regInfo.baseInfo.from.SetHeader(request->from->url->username,
                                    request->from->url->host, 
                                    request->from->url->port);
    regInfo.baseInfo.proxy.SetHeader(request->to->url->username,
                                     request->to->url->host, 
                                     request->to->url->port);

    //��ȡexpires
    osip_header_t* header = NULL;
    {
        osip_message_header_get_byname(request, "expires", 0, &header);
        if (NULL != header && NULL != header->hvalue)
        {
            regInfo.baseInfo.expires = atoi(header->hvalue);
        }
    }

    //contact�ֶ�
    osip_contact_t* contact = NULL;
    osip_message_get_contact(request, 0, &contact);
    if (NULL != contact)
    {
        regInfo.baseInfo.contact.SetContractHeader(contact->url->username,
                                                   contact->url->host, 
                                                   contact->url->port,
                                                   regInfo.baseInfo.expires);
    }

    //via�ֶ�
    osip_via_t* via = NULL;
    osip_message_get_via(request, 0, &via);
    if (NULL != contact)
    {
    }

    //ע�᷵�� �ɷ��ͷ�ά��������ID ���շ����պ�ԭ�����ؼ���
    regInfo.baseInfo.sipRequestId = iReqId;

    //CALL_ID
    {
        stream.str("");
        stream << request->call_id->number;
        regInfo.baseInfo.callId = stream.str();
    }

    //����content��Ϣ
    osip_body_t * body = NULL;
    osip_message_get_body(request, 0, &body);
    if (body != NULL)
    {
        stream.str("");
        stream << body->body;
        regInfo.baseInfo.content = stream.str();
    }

    //��Ȩ��Ϣ
    osip_authorization_t* authentication = NULL;
    {
        osip_message_get_authorization(request, 0, &authentication);
        if (NULL == authentication)
        {
            regInfo.isAuthNull = true;
        }
        else
        {
            regInfo.isAuthNull = false;
            stream.str("");
            stream << authentication->username;
            regInfo.authInfo.userName = stream.str();
            stream.str("");
            stream << authentication->algorithm;
            regInfo.authInfo.algorithm = stream.str();
            stream.str("");
            stream << authentication->realm;
            regInfo.authInfo.digestRealm = stream.str();
            stream.str("");
            stream << authentication->nonce;
            regInfo.authInfo.nonce = stream.str();
            stream.str("");
            stream << authentication->response;
            regInfo.authInfo.response = stream.str();
            stream.str("");
            stream << authentication->uri;
            regInfo.authInfo.uri = stream.str();
        }
    }
    authentication = NULL;
}

void CSipRegister::printRegisterPkt(sipRegisterInfo& info)
{
    LogDebug("���յ����ģ�");
    LogDebug("================================================================");
    LogDebug("method:      %s",info.baseInfo.method.c_str());
    LogDebug("from:        %s",info.baseInfo.from.GetFormatHeader().c_str());
    LogDebug("to:          %s",info.baseInfo.proxy.GetFormatHeader().c_str());
    LogDebug("contact:     %s",info.baseInfo.contact.GetContractFormatHeader(false).c_str());

    //ע�᷵�� �ɷ��ͷ�ά��������ID ���շ����պ�ԭ�����ؼ���
    LogDebug("RequestId:   %d",info.baseInfo.sipRequestId);
    //CALL_ID
    LogDebug("CallId:      %s",info.baseInfo.callId.c_str());
    //����content��Ϣ
    LogDebug("body:        %s",info.baseInfo.content.c_str());
    //��ȡexpires
    LogDebug("expires:     %d",info.baseInfo.expires);
    //��Ȩ��Ϣ
    if (info.isAuthNull)
    {
        LogDebug("��ǰ����δ�ṩ��Ȩ��Ϣ!!!");
    }
    else
    {
        LogDebug("��ǰ���ļ�Ȩ��Ϣ����:");
        LogDebug("username: %s",info.authInfo.userName.c_str());
        LogDebug("algorithm:%s",info.authInfo.algorithm.c_str());
        LogDebug("Realm:    %s",info.authInfo.digestRealm.c_str());
        LogDebug("nonce:    %s",info.authInfo.nonce.c_str());
        LogDebug("response: %s",info.authInfo.response.c_str());
        LogDebug("uri:      %s",info.authInfo.uri.c_str());
    }
    LogDebug("================================================================");
    return;
}

void CSipRegister::sendRegisterAnswer(sipRegisterInfo& info)
{
    osip_message_t* answer = NULL;
    int iStatus;
    eXosip_lock(m_pExContext);
    if (m_bAuthorization && info.isAuthNull)
    {
        iStatus = 401;
        int result = ::eXosip_message_build_answer(m_pExContext,info.baseInfo.sipRequestId,iStatus, &answer);
        if (OSIP_SUCCESS != result)
        {
            ::eXosip_message_send_answer(m_pExContext,info.baseInfo.sipRequestId, 400, NULL);
        }
        else
        {
            std::stringstream stream;
            string nonce = m_strNonce;
            string algorithm = m_strAlgorithm;
            stream << "Digest realm=\"" << info.baseInfo.from.GetRealName() 
                << "\",nonce=\"" << nonce
                << "\",algorithm=" << algorithm;

            osip_message_set_header(answer, "WWW-Authenticate", stream.str().c_str());
            ::eXosip_message_send_answer(m_pExContext,info.baseInfo.sipRequestId, iStatus, answer);
            Log::warning("ע��Ӧ�� ����401����");
        }
    }
    else
    {
        iStatus = 200;
        int result = ::eXosip_message_build_answer(m_pExContext,info.baseInfo.sipRequestId,iStatus, &answer);
        if (OSIP_SUCCESS != result)
        {
            ::eXosip_message_send_answer(m_pExContext,info.baseInfo.sipRequestId, 400, NULL);
             Log::warning("ע��Ӧ�� ����400����");
        }
        else
        {
            osip_message_set_header(answer, "Contact", info.baseInfo.contact.GetContractFormatHeader(true).c_str());
            //������Ϣ��
            ::eXosip_message_send_answer(m_pExContext,info.baseInfo.sipRequestId, iStatus, answer);
            Log::warning("ע��Ӧ�� ����200����");

            //ע��ɹ����豸����ӵ��豸����
            PlatFormInfo pNewRegist;
            pNewRegist.strDevCode   = info.baseInfo.contact.GetAddrCode();
            pNewRegist.strAddrIP    = info.baseInfo.contact.GetRealName();
            pNewRegist.strAddrPort  = info.baseInfo.contact.GetPort();
            pNewRegist.nExpire      = info.baseInfo.expires;
            pNewRegist.strStatus    = "1";
            DeviceMgr::RegistPlatform(pNewRegist);

            // ��ѯĿ¼��Ϣ
            //if (CSipMgr::m_pMessage != nullptr)
            //{
                //CSipMgr::m_pMessage->QueryDirtionary(info.baseInfo.contact.GetAddrCode(),info.baseInfo.contact.GetRealName(),info.baseInfo.contact.GetPort());
                //Log::debug(" QueryDirtionary %s",info.baseInfo.contact.GetAddrCode().c_str());
				//CSipMgr::m_pMessage->QueryDeviceStatus(info.baseInfo.contact.GetAddrCode(),info.baseInfo.contact.GetRealName(),info.baseInfo.contact.GetPort(),"36030100061320000028");
				//CSipMgr::m_pMessage->QueryDeviceInfo(info.baseInfo.contact.GetAddrCode(),info.baseInfo.contact.GetRealName(),info.baseInfo.contact.GetPort(),"36030100061320000028");
				//CSipMgr::m_pMessage->QueryMobilePosition(info.baseInfo.contact.GetAddrCode(),info.baseInfo.contact.GetRealName(),info.baseInfo.contact.GetPort(),"36030100061320000028");
				//CSipMgr::m_pMessage->QueryRecordInfo(pNewRegist->strDevCode, pNewRegist->strAddrIP, pNewRegist->strAddrPort, "36030100061320000001", "2018-08-30T00:00:00", "2018-09-02T23:59:59");
            //}
			
			//if (CSipMgr::m_pSubscribe != nullptr)
			//{
			//	CSipMgr::m_pSubscribe->Subscribe(info.baseInfo.contact.GetAddrCode(),info.baseInfo.contact.GetRealName(),info.baseInfo.contact.GetPort());
			//	CSipMgr::m_pSubscribe->SubscribeMobilepostion(info.baseInfo.contact.GetAddrCode(),info.baseInfo.contact.GetRealName(),info.baseInfo.contact.GetPort());
			//	Log::debug(" Subscribe %s",info.baseInfo.contact.GetAddrCode().c_str());
			//}
			
        }
    }


    if (0 == info.baseInfo.expires)
    {
        eXosip_register_remove(m_pExContext,info.baseInfo.sipRequestId);
    }

    eXosip_unlock(m_pExContext);
}