#ifndef _SCRIBE_PROXY_HPP_
#define _SCRIBE_PROXY_HPP_
#include "common.h"
#include "gen-cpp/scribe.h"
#include <vector>
#include "scribe_handler.h"
#include <utils/implement_base.h>
#include "utils/task/stask_base.h"

using std::string;
using std::ostringstream;
using std::map;


using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace facebook::fb303;
using namespace facebook;

using namespace scribe::thrift;
using namespace std;

using boost::shared_ptr;


#define MAX_QUEUE_LEN 100

struct queue_node{
	std::vector<std::string> keys;
	std::string value;
};


class ScribeProxyClient:public STaskBase<queue_node>
{
public:
	ScribeProxyClient(){};
	virtual ~ScribeProxyClient(){};
	virtual int Svc();


	int Init(const std::string& remote_addr,int port,int timeout,int buf_len=1);
	int Open();
	int Send(std::vector<std::string>& key,const std::string& value);

	int _open();
	int _send(std::vector<std::string>& key,const std::string& value);
	void _close();
	int Close();
protected:
	boost::shared_ptr<apache::thrift::transport::TSocket> socket;
	boost::shared_ptr<apache::thrift::transport::TFramedTransport> framedTransport;
	boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> protocol;
	boost::shared_ptr<scribe::thrift::scribeClient> resendClient;

	std::string remote_addr;
	int port;
	int time_out;
	std::vector<LogEntry> msgs_;
	int buffer_len_;
	bool flag_open;
};


class ScribeProxyServer
{
public:
	ScribeProxyServer(ImplementBase* impl){impl_=impl;};

	//if category="",you want to receiver all category
	int Init(int port,std::string category,int thread_num,int connection_num);
	void Open();
	void Close();

protected:
	shared_ptr<scribeHandler> g_Handler;
	boost::shared_ptr<TProcessor> m_processor;
	boost::shared_ptr<TProtocolFactory> m_protocol_factory;
	boost::shared_ptr<ThreadManager> m_thread_manager;
	shared_ptr<TNonblockingServer> m_server;

	int m_port;
	int m_thread_num;
	int m_conn_num;
	ImplementBase* impl_;
	std::string category_;
};

#endif
