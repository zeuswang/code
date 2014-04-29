#include "scribe_proxy.h"
#include <Platform/log.h>
//#include "handle_task.hpp"
SS_LOG_MODULE_DEF(ods_utils);

int ScribeProxyClient::Init(const std::string& _addr,int _port,int timeout,int buf_len)
{
	remote_addr =_addr ;
	port =_port;
	time_out= timeout;
	buffer_len_= buf_len;
	return 0;
}

int ScribeProxyClient::_open(){
	try 
	{
        socket = shared_ptr<TSocket>(new TSocket(remote_addr,port));
        if (!socket) {
            throw std::runtime_error("Failed to create socket");
        }
        int timeout =time_out;
        socket->setConnTimeout(timeout);
        socket->setRecvTimeout(timeout);
        socket->setSendTimeout(timeout);

        framedTransport = shared_ptr<TFramedTransport>(new TFramedTransport(socket));
        if (!framedTransport) {
            throw std::runtime_error("Failed to create framed transport");
        }
        protocol = shared_ptr<TBinaryProtocol>(new TBinaryProtocol(framedTransport));
        if (!protocol) {
            throw std::runtime_error("Failed to create protocol");
        }
        protocol->setStrict(false, false);
        resendClient = shared_ptr<scribeClient>(new scribeClient(protocol));
        if (!resendClient) {
            throw std::runtime_error("Failed to create network client");
        }

        framedTransport->open();    
        flag_open=true;//Õý³£^M
        return 1;
    } catch (TTransportException& ttx) {
        flag_open=false;
        fprintf(stderr,"scribe_client open error\n");
        return 0;
    } catch (std::exception& stx) {
        flag_open=false;
        fprintf(stderr,"scribe_client open error\n");
        return 0;
    }
	return 0;
}

int ScribeProxyClient::Open(){
	
	_open();	
	int ret2 =STaskBase<queue_node>::Open(1);
	if (ret2)
	{
		return 0;
	}
	Activate();
	return 1;
}

int ScribeProxyClient::Svc(){
	queue_node msg;
	fprintf(stderr,"[scribe_task::svc]begin to loop\n");
	while (!Get(msg))
	{
		if(_send(msg.keys,msg.value)){
			fprintf(stderr,"[scribe_task::svc]send scribe Error\n");			
			continue;
		}
		fprintf(stderr,"[scribe_task::svc]send scribe sucess,que_len:%d\n",GetCount());
	}
	fprintf(stderr,"[scribe_task::svc]exit svc loop\n");
	return 0;
}

int ScribeProxyClient::Send(std::vector<std::string>& key,const std::string& value){
	queue_node msg;
	msg.keys=key;
	msg.value=value;

	if (GetCount()>=MAX_QUEUE_LEN)
	{
		WEBSEARCH_DEBUG((LM_ERROR,"[ScribeProxyClient::Put]too many!content:%s\n",msg.value.c_str()));
		return -1;
	}
	STaskBase<queue_node>::Put(msg);
	return 0;
}

int ScribeProxyClient::_send(std::vector<std::string>& key_list,const std::string& value)
{
	msgs_.clear();
	for (std::vector<std::string>::iterator iter = key_list.begin();iter != key_list.end();++iter) {
		LogEntry log_entry;
		log_entry.category=*iter;
		log_entry.message = value;
		msgs_.push_back(log_entry);
	}

	ResultCode result = TRY_LATER;
	//result = resendClient->Log(msgs);
	try {
		result = resendClient->Log(msgs_);
		if (result == OK) {
			return 0;
		} else {
			WEBSEARCH_DEBUG((LM_ERROR,"[scribeClient]Failed to send messages, returned error code <%d>\n",(int) result));
			return -1; // Don't retry here. If this server is overloaded they probably all are.
		}
	} catch (TTransportException& ttx) {
		WEBSEARCH_DEBUG((LM_ERROR,"[scribeClient]throw TTransportException exception,  error code <%d>\n",(int) result));

	} catch (...) {
		WEBSEARCH_DEBUG((LM_ERROR,"[scribeClient]Unknown exception sending messages to remote scribe server!\n",(int) result));
	}
		// we only get here if the send threw an exception
	_close();
	if (_open()) {
		WEBSEARCH_DEBUG((LM_DEBUG,"[scribeClient]reopened connection to remote scribe server"));
	} else {
		WEBSEARCH_DEBUG((LM_DEBUG,"[scribeClient]reopened connection to remote scribe server error!"));
		return -1;
	}
	return 0;
}

void ScribeProxyClient::_close(){
	try {
		framedTransport->close();
	} catch (TTransportException& ttx) {
		WEBSEARCH_DEBUG((LM_DEBUG,"error <%s> while closing connection to remote scribe server",
			ttx.what()));    
	}
}
int ScribeProxyClient::Close() {
    STaskBase<queue_node>::Close();
	_close();
	return 0;
}

int ScribeProxyServer::Init(int port,std::string category,int t_num,int c_num)
{
	m_port = port;
	m_thread_num = t_num;
	m_conn_num = c_num;
	category_= category;
	return 0;
}
void ScribeProxyServer::Open()
{

try{
	struct rlimit r_fd = {65535,65535};
	if (-1 == setrlimit(RLIMIT_NOFILE, &r_fd)) {
		fprintf(stderr,"setrlimit error (setting max fd size)\n");
	}
	srand(time(NULL) ^ getpid());

	g_Handler = shared_ptr<scribeHandler>(new scribeHandler(m_port));
	g_Handler->setImplementbase(impl_);
  	g_Handler->setCategory(category_);
	g_Handler->initialize();

	m_processor=shared_ptr<TProcessor>(new scribeProcessor(g_Handler));
	// This factory is for binary compatibility. 
	m_protocol_factory=shared_ptr<TProtocolFactory> (
		new TBinaryProtocolFactory(0, 0, false, false)
	);

	int numThriftServerThreads = m_thread_num;
	{
		// create a ThreadManager to process incoming calls
		m_thread_manager = ThreadManager::newSimpleThreadManager(numThriftServerThreads);

		shared_ptr<PosixThreadFactory> thread_factory(new PosixThreadFactory());
		m_thread_manager->threadFactory(thread_factory);
		m_thread_manager->start();
	}

	m_server=shared_ptr<TNonblockingServer>(new TNonblockingServer(
	                  m_processor,
	                  m_protocol_factory,
	                  g_Handler->port,
	                  m_thread_manager
    ));
	// throttle concurrent connections
	int mconn = m_conn_num;
	m_server->setMaxConnections(mconn);
	//m_server->setOverloadAction(T_OVERLOAD_CLOSE_ON_ACCEPT);
	m_server->setOverloadAction(T_OVERLOAD_NO_ACTION);
	m_server->serve();
}catch(std::exception& stx) {
	WEBSEARCH_DEBUG((LM_ERROR,"ScribeProxyServer::Open() error!\n"));
}
}
void ScribeProxyServer::Close()
{
	struct event_base* event_base_ = m_server->getEventBase();
	if (event_base_)
		event_base_loopexit(event_base_, NULL);
}
