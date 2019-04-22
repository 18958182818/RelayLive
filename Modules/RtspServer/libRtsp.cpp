#include "stdafx.h"
#include "libRtsp.h"

namespace RtspServer
{
typedef enum _parse_step_
{
    parse_step_method = 0, //δ��ʼ,��������� [OPIONS��DESCRIBE��SETUP��PLAY��TEARDOWN]
    parse_step_uri,        //�����uri
    parse_step_protocol,   //�����Э��[rtsp]
    parse_step_version,    //������汾
    parse_step_header_k,   //���������ͷ�ֶε�key
    parse_step_header_v    //���������ͷ�ֶε�value
}parse_step_t;

char* response_status[] = {
    "100 Continue(all 100 range)",
    "110 Connect Timeout",
    "200 OK",
    "201 Created",
    "250 Low on Storage Space",
    "300 Multiple Choices",
    "301 Moved Permanently",
    "302 Moved Temporarily",
    "303 See Other",
    "304 Not Modified",
    "305 Use Proxy",
    "350 Going Away",
    "351 Load Balancing",
    "400 Bad Request",
    "401 Unauthorized",
    "402 Payment Required",
    "403 Forbidden",
    "404 Not Found",
    "405 Method Not Allowed",
    "406 Not Acceptable",
    "407 Proxy Authentication Required",
    "408 Request Time-out",
    "410 Gone",
    "411 Length Required",
    "412 Precondition Failed",
    "413 Request Entity Too Large",
    "414 Request-URI Too Large",
    "415 Unsupported Media Type",
    "451 Parameter Not Understood",
    "452 reserved",
    "453 Not Enough Bandwidth",
    "454 Session Not Found",
    "455 Method Not Valid in This State",
    "456 Header Field Not Valid for Resource",
    "457 Invalid Range",
    "458 Parameter Is Read-Only",
    "459 Aggregate operation not allowed",
    "460 Only aggregate operation allowed",
    "461 Unsupported transport",
    "462 Destination unreachable",
    "500 Internal Server Error",
    "501 Not Implemented",
    "502 Bad Gateway",
    "503 Service Unavailable",
    "504 Gateway Time-out",
    "505 RTSP Version not supported",
    "551 Option not supported"
};
    
static void on_close(uv_handle_t* peer) {
    CClient* client = (CClient*)peer->data;
	client->m_server->m_options.cb(client, RTSP_CLOSE, client->m_user);
}

static void after_shutdown(uv_shutdown_t* req, int status) {
    uv_close((uv_handle_t*)req->handle, on_close);
    free(req);
}

static void echo_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

static void after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    CClient* client = (CClient*)handle->data;
    if (nread < 0) {
        if(nread == UV_EOF){
            Log::debug("remote close this socket");
        } else {
            Log::debug("other close %s",  uv_strerror(nread));
        }

        if (buf->base) {
            free(buf->base);
        }

        uv_shutdown_t* req = (uv_shutdown_t*) malloc(sizeof(uv_shutdown_t));
        uv_shutdown(req, handle, after_shutdown);

        return;
    }

    if (nread == 0) {
        /* Everything OK, but nothing read. */
        free(buf->base);
        return;
    }

    rtsp_ruquest req = client->parse(buf->base, nread);

    client->answer(req);
}

static void after_write(uv_write_t* req, int status) {
    if (status < 0)
    {
        Log::error("after_write fail:%s", uv_strerror(status));
    }
}

static void on_async(uv_async_t* handle){
    CClient *c = (CClient*)handle->data;
    uv_mutex_lock(&c->m_asyncMutex);
    while(!c->m_asyncList.empty()){
        rtsp_event e = c->m_asyncList.front();
        c->m_server->m_options.cb(c, e.resaon, c->m_user, NULL, 0);
        c->m_asyncList.pop_front();
    }
    uv_mutex_unlock(&c->m_asyncMutex);
}

CClient::CClient()
    : m_ploop(nullptr)
    , m_server(nullptr)
    , m_Request(nullptr)
    , m_Response(nullptr)
{
}

CClient::~CClient()
{
	SAFE_FREE(m_user);
	 uv_mutex_destroy(&m_asyncMutex);
	 m_server->GiveBackRtpPort(m_nLocalPort);
}

int CClient::Init(uv_loop_t* uv) {
    m_ploop = uv;
    int r = uv_tcp_init(m_ploop, &m_rtsp);
    if(r < 0) {
        Log::error("client init rtsp error %s",  uv_strerror(r));
        return r;
    }
    r = uv_udp_init(m_ploop, &m_rtp);
    if(r < 0) {
        Log::error("client init rtp error %s", uv_strerror(r));
        return r;
    }
	m_nLocalPort = m_server->GetRtpPort();
	struct sockaddr_in addr;
    r = uv_ip4_addr(m_server->m_options.ip.c_str(), m_nLocalPort, &addr);
    if(r < 0) {
        Log::error("make address err: %s",  uv_strerror(r));
        return -1;
    }
	r = uv_udp_bind(&m_rtp, (struct sockaddr*)&addr, 0);
    if(r < 0) {
        Log::error("client init bind rtp error %s", uv_strerror(r));
        return r;
    }
    r = uv_udp_init(m_ploop, &m_rtcp);
    if(r < 0) {
        Log::error("client init rtcp error %s", uv_strerror(r));
        return r;
    }
    r = uv_async_init(m_ploop, &m_async, on_async);
    if(r < 0) {
        Log::error("client init rtcp error %s", uv_strerror(r));
        return r;
    }
    r = uv_mutex_init(&m_asyncMutex);
    if(r < 0) {
        Log::error("client init mutex error %s", uv_strerror(r));
        return r;
    }
    m_user = malloc(m_server->m_options.user_len);
    if(NULL == m_user){
        Log::error("client init malloc error");
        return 1;
    }
	memset(m_user, 0, m_server->m_options.user_len);

    m_rtsp.data  = (void*)this;
    m_rtp.data   = (void*)this;
    m_rtcp.data  = (void*)this;
    m_async.data = (void*)this;
    return 0;
}

int CClient::Recv() {
    int r = uv_read_start((uv_stream_t*)&m_rtsp, echo_alloc, after_read);
    if (r < 0)
    {
        Log::error("read start error %s",  uv_strerror(r));
        return r;
    }
    return 0;
}

rtsp_ruquest CClient::parse(char* buff, int len) {
    rtsp_ruquest ret;
    ret.method = RTSP_ERROR;
    ret.parse_status = true;
    parse_step_t step =  parse_step_method;
    string tmp,tmp2;
    for (int i = 0; i < len; ++i) {
        if (parse_step_method == step)
        {
            if(buff[i] == ' ') {
                Log::debug("request method is %s", tmp.c_str());
                if(_stricmp(tmp.c_str(), "OPTIONS") == 0){
                    ret.method = RTSP_OPTIONS;
                } else if(_stricmp(tmp.c_str(), "DESCRIBE") == 0){
                    ret.method = RTSP_DESCRIBE;
                }else if(_stricmp(tmp.c_str(), "SETUP") == 0){
                    ret.method = RTSP_SETUP;
                }else if(_stricmp(tmp.c_str(), "PLAY") == 0){
                    ret.method = RTSP_PLAY;
                }else if(_stricmp(tmp.c_str(), "PAUSE") == 0){
                    ret.method = RTSP_PAUSE;
                }else if(_stricmp(tmp.c_str(), "TEARDOWN") == 0){
                    ret.method = RTSP_TEARDOWN;
                } else {
                    Log::error("error method : %s", tmp.c_str());
                    ret.parse_status = false;
                    ret.code = Code_551_OptionNotSupported;
                    break;
                }
                tmp.clear();
                step = parse_step_uri;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_uri == step) {
            if (buff[i] == ' ') {
                ret.uri = tmp;
                tmp.clear();
                step = parse_step_protocol;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_protocol == step) {
            if (buff[i] == '/') {
                if(_stricmp(tmp.c_str(), "RTSP") != 0) {
                    Log::error("error protocol : %s", tmp.c_str());
                    ret.parse_status = false;
                    ret.code = Code_400_BadRequest;
                    break;
                }
                tmp.clear();
                step = parse_step_version;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_version == step) {
            if (buff[i] == '\r' && buff[i+1] == '\n') {
                if(_stricmp(tmp.c_str(), "1.0") != 0) {
                    Log::error("error version : %s", tmp.c_str());
                    ret.parse_status = false;
                    ret.code = Code_505_RtspVersionNotSupported;
                    break;
                }
                tmp.clear();
                i++;
                step = parse_step_header_k;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_header_k == step) {
            if (buff[i] == ':') {
                step = parse_step_header_v;
            } else {
                tmp.push_back(buff[i]);
            }
        } else if (parse_step_header_v == step) {
            if (buff[i] == '\r' && buff[i+1] == '\n') {
                ret.headers.insert(make_pair(tmp,tmp2));
                tmp.clear();
                tmp2.clear();
                i++;
                step = parse_step_header_k;
            } else {
                if(!tmp2.empty() || buff[i] != ' ')
                    tmp2.push_back(buff[i]);
            }
        }
    }

    return ret;
}

int CClient::answer(rtsp_ruquest req)
{
    rtsp_response res;
    m_Request = &req;
    m_Response = &res;

    do{
        auto seq_it = req.headers.find("CSeq");
        if (seq_it == req.headers.end()) {
            res.code = Code_400_BadRequest;
            break;
        }
        try {
            req.CSeq = stoi(seq_it->second);
        } catch (...) {
            res.code = Code_400_BadRequest;
            break;
        }
        if (!req.parse_status) {
            res.code = req.code;
            break;
        }

        if(req.method == rtsp_method::RTSP_OPTIONS) {
            int ret = m_server->m_options.cb(this, req.method, m_user);
            if(ret)
                break;
        } else if(req.method == rtsp_method::RTSP_DESCRIBE) {
            int ret = m_server->m_options.cb(this, req.method, m_user);
            if(ret)
                break;
        } else if(req.method == rtsp_method::RTSP_SETUP) {
            int ret = m_server->m_options.cb(this, req.method, m_user);
            if(ret)
                break;
			
			string session = StringHandle::toStr<uint64_t>(CRtspServer::m_nSession++);
			res.headers.insert(make_pair("Session",session));
        } else if(req.method == rtsp_method::RTSP_PLAY) {
            int ret = m_server->m_options.cb(this, req.method, m_user);
            if(ret)
                break;
        } else if(req.method == rtsp_method::RTSP_PAUSE) {
            int ret = m_server->m_options.cb(this, req.method, m_user);
            if(ret)
                break;
        } else if(req.method == rtsp_method::RTSP_TEARDOWN) {
            int ret = m_server->m_options.cb(this, req.method, m_user);
            if(ret)
                break;
        } else {
            res.code = Code_551_OptionNotSupported;
        }
        res.CSeq = req.CSeq;
    }while(0);

    //����tcpӦ����
    char time_buff[50]={0};
    time_t time_now = time(NULL);
    ctime_s(time_buff, 49, &time_now);
    string strTime = time_buff;
    strTime = strTime.substr(0,strTime.size()-1);
    strTime += " GMT";
    stringstream ss;
    ss << "RTSP/1.0 " << response_status[res.code] << "\r\n"
        << "CSeq: " << res.CSeq << "\r\n"
        << "Date: " << strTime << "\r\n";
    for (auto& h:res.headers)
    {
        ss << h.first << ": " << h.second << "\r\n";
    }
    if (!res.body.empty())
    {
        ss << "Content-Length: " << res.body.size()+2 << "\r\n\r\n";
        ss << res.body;
    }
    ss << "\r\n";
    string strResponse = ss.str();

    //����Ӧ��
    uv_write_t *wr = (uv_write_t*)malloc(sizeof(uv_write_t));
    wr->data = this;
    uv_buf_t buff = uv_buf_init((char*)strResponse.c_str(), strResponse.size());
    int ret = uv_write(wr, (uv_stream_t*)&m_rtsp,&buff, 1, after_write);
    if (ret < 0)
    {
        Log::error("uv_write fail:%s",  uv_strerror(ret));
    }
    return 0;
}

void CClient::SetRemotePort(int rtp, int rtcp)
{
    m_nRtpPort = rtp;
    m_nRtcpPort = rtcp;
    int ret = uv_ip4_addr(m_strRtpIP.c_str(), rtp, &m_addrRtp);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return ;
    }
}

//////////////////////////////////////////////////////////////////////////

volatile uint64_t CRtspServer::m_nSession = 11111111;

static void on_connection(uv_stream_t* server, int status) {
    if (status != 0) {
        Log::error("Connect error %s",  uv_strerror(status));
        return;
    }

    CRtspServer* rtsp = (CRtspServer*)server->data;

    CClient* client = new CClient;
    client->m_server = rtsp;
    int r = client->Init(rtsp->m_ploop);
    if(r < 0) {
        Log::error("client init error %s",  uv_strerror(r));
        return;
    }

    r = uv_accept(server, (uv_stream_t*)&client->m_rtsp);
    if(r < 0) {
        Log::error("accept error %s",  uv_strerror(status));
        return;
    }

	// �ͻ���IP
	struct sockaddr_in addr;
	char ipv4addr[64];
	int namelen = sizeof(addr);
	uv_tcp_getpeername(&client->m_rtsp, (struct sockaddr*)&addr, &namelen);
	uv_ip4_name(&addr, ipv4addr, 64);
	client->m_strRtpIP = ipv4addr;

	// ����IP
	uv_tcp_getsockname(&client->m_rtsp, (struct sockaddr*)&addr, &namelen);
	uv_ip4_name(&addr, ipv4addr, 64);
	client->m_strLocalIP = ipv4addr;

    r = client->Recv();
    if (r < 0)
    {
        Log::error("read start error %s",  uv_strerror(r));
        return;
    }
}

CRtspServer::CRtspServer(rtsp_options options)
    : m_options(options)
{
	
	for (int i=0; i<options.rtp_port_num; ++i) {
        m_vecRtpPort.push_back(options.rtp_port+i*2);
    }
}

CRtspServer::~CRtspServer(void)
{
}

int CRtspServer::Init(uv_loop_t* uv)
{
    m_ploop = uv;
    uv_tcp_init(m_ploop, &m_tcp);

    struct sockaddr_in addr;
    int ret = uv_ip4_addr(m_options.ip.c_str(), m_options.port, &addr);
    if(ret < 0) {
        Log::error("make address err: %s",  uv_strerror(ret));
        return -1;
    }

    ret = uv_tcp_bind(&m_tcp, (struct sockaddr*)&addr, 0);
    if(ret < 0) {
        Log::error("tcp bind err: %s",  uv_strerror(ret));
        return -1;
    }

    m_tcp.data = (void*)this;
    ret = uv_listen((uv_stream_t*)&m_tcp, SOMAXCONN, on_connection);
    if (ret < 0)
    {
        Log::error("uv listen err:%s", uv_strerror(ret));
        return -1;
    }

	Log::debug("rtsp server init success");
    return 0;
}

int  CRtspServer::GetRtpPort()
{
    MutexLock lock(&m_csRTP);

    int nRet = -1;
    auto it = m_vecRtpPort.begin();
    if (it != m_vecRtpPort.end()) {
        nRet = *it;
        m_vecRtpPort.erase(it);
    }

    return nRet;
}

void  CRtspServer::GiveBackRtpPort(int nPort)
{
    MutexLock lock(&m_csRTP);
    m_vecRtpPort.push_back(nPort);
}

//////////////////////////////////////////////////////////////////////////

int rtp_callback_on_writable(CClient *client){
    uv_mutex_lock(&client->m_asyncMutex);
    rtsp_event e = {RTP_WRITE};
    client->m_asyncList.push_back(e);
    uv_mutex_unlock(&client->m_asyncMutex);
    uv_async_send(&client->m_async);
    return 0;
}

static void on_udp_send(uv_udp_send_t* req, int status){
    SAFE_FREE(req->data);
    SAFE_FREE(req);
}

int rtp_write(CClient *client, char* buff, int len){
    SAFE_MALLOC(uv_udp_send_t, req);
    uv_buf_t buf = uv_buf_init(buff, len);
    req->data = buff;
    int ret = uv_udp_send(req, &client->m_rtp, &buf, 1, (const sockaddr*)&client->m_addrRtp, on_udp_send);
    if(ret != 0){
        Log::debug("udp send failed %d: %s", ret, uv_err_name((ret)));
        return ret;
    }
    return 0;
}

}