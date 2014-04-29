#include "connection_client.h"
#include <utils/debug.h>
int tcp_check::Svc()
{
 	_tcp_handler->CheckRoutine();
	return 0;
}

ConnectionClient::ConnectionClient(ImplementBase* base)
{
	_stop_task = 0;
	max_fd_num_ = 1024;
	_epoll_fd = INVALID_FD;
	_epoll_ready_event_num = 0;
	_pipe_read = INVALID_FD;
	_pipe_write = INVALID_FD;
	_update_record_num = 0;

	pthread_mutex_init(&_epoll_mutex, NULL);
	pthread_mutex_init(&_fds_update_mutex, NULL);
	pthread_mutex_init(&_err_connect_mutex,NULL);

	impl_base_ = base;

}
ConnectionClient::~ConnectionClient()
{
	::close(_epoll_fd);
	::close(_pipe_read);
	::close(_pipe_write);
	pthread_mutex_destroy(&_epoll_mutex);
	pthread_mutex_destroy(&_fds_update_mutex);
	pthread_mutex_destroy(&_err_connect_mutex);

}
int  ConnectionClient::Close()
{
	_stop_task = 1;
	write(_pipe_write, &_stop_task, 1);
	//stask_base::stop();

	// delete check routine
	if (_check){
		_check->Close();
		delete _check;
	}	

	if (topology_)
		delete topology_;
	
	STaskBase<Connection*>::Close();
	return 0;	
}
int ConnectionClient::LoadConf(const std::string& conf)
{

	//TopologyFactory t_factory;
	//topology_ = t_factory.GetNewTopology(T_DocidCircle);
	topology_ = new DocidCircleTopology();

	int pos=0;
	while(1){

		int pos2=conf.find(";",pos);
		if (pos2<0)
			break;
		int pos3=conf.find(":",pos);
		std::string ip=conf.substr(pos,pos3-pos);
		std::string port=conf.substr(pos3+1,pos2-pos3-1);
		//WEBSEARCH_DEBUG((LM_DEBUG,"ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str())));

		//add a node to topology_
		NodeTopology* node_topology=new NodeTopology();
		node_topology->GetVNode()->SetAddress(ip,port);
		Connection* conn = node_topology->GetVNode()->CreateNewConnection();
		if (conn){
			/*if (conn->set_socket(conn->GetFd(), O_NONBLOCK)){
				fprintf(stderr,"ConnectionClient::LoadConf set_socket error!\n");
				return -1;
			}*/
			if (conn->ConnectTimeout(ip,port,50)==0){
    			connect_set.insert(std::map<int,Connection*>::value_type(conn->GetFd(),conn));	
    			conn->SetFlag(READABLE_SOCKET | WRITABLE_SOCKET |SERVER_SOCKET);
    			add_input_fd(conn->GetFd());
			}
    			conn->SetConnectionManager(this);
    			topology_->AddChild(node_topology);
    			//topology_->GetList().push_back(node_topology);
    			conn->SetVNode(node_topology->GetVNode());
			
		}
		pos=pos2+1;
	}
	return 0;
}


Connection * ConnectionClient::SelectConnection(const std::string& key)
{
	//pthread_mutex_lock(&_err_connect_mutex);
	//Connection* conn= topology_->SelectConnection(key);
	//if (conn)
	//	conn->acquire();
	Connection* conn= topology_->SelectConnection(key);
  conn->LockSelf();
  if (conn->GetFd() <0){
    _debug("get conn with invalid fd\n");
    conn->UnLockSelf();
    //	pthread_mutex_unlock(&_err_connect_mutex);
    	return NULL;
  }else{
    conn->UnLockSelf();
    //pthread_mutex_unlock(&_err_connect_mutex);
    return conn;
  }
}

void ConnectionClient::CheckConnection(Topology* tpy)
{
	TopologyType type = tpy->GetType();
	
	if (type == T_Node){
		std::vector<Connection*>& clist = tpy->GetVNode()->GetList();
		for (size_t i=0;i<clist.size();i++){
			Connection * conn = clist[i];
			if (!conn)
				continue;
			
			conn->LockSelf();
			if (conn->GetFd() >=0){
				conn->UnLockSelf();
				continue;
			}

			if (conn->ConnectTimeout(tpy->GetVNode()->GetAddress(),tpy->GetVNode()->GetPort(),50)<0){
				conn->UnLockSelf();
				continue;
			}
			conn->SetFlag(READABLE_SOCKET | WRITABLE_SOCKET |SERVER_SOCKET);
			
			pthread_mutex_lock(&_err_connect_mutex);
			connect_set.insert(std::map<int,Connection*>::value_type(conn->GetFd(),conn));	
			pthread_mutex_unlock(&_err_connect_mutex);	


			add_input_fd_record(conn->GetFd());
			_debug("found Connection error and reconnect it \n");
			conn->UnLockSelf();
			
		}

	}else{
		std::vector<Topology*>& list = tpy->GetList();
		for (size_t i=0;i<list.size();i++){
			CheckConnection(list[i]);
		}
	}
}
int ConnectionClient::CheckRoutine()
{
	_stop_task= 0;
	timeval timeout;
	while (!_stop_task)
	{
		timeout.tv_sec = m_CheckInter;
		timeout.tv_usec = 0;
		if (select(1, NULL, NULL, NULL, &timeout)) continue;
		CheckConnection(topology_);
	}
	return 0;
}
void ConnectionClient::ErrorHandle(Connection* conn)
{

	if(conn)
	{
		conn->LockSelf();
		pthread_mutex_lock(&_err_connect_mutex);
    int fd = conn->GetFd();
    _debug("ErrorHandle,fd=%d\n",fd);
		connect_set.erase(fd);
		pthread_mutex_unlock(&_err_connect_mutex);
		conn->Close();	
		conn->UnLockSelf();
	}
	/*

	if(conn)
	{

		//conn->SetFlag(conn->GetFlag()&~SERVER_SOCKET&~READABLE_SOCKET);
		//if(conn->release() == 0)
		//{	
			pthread_mutex_lock(&_err_connect_mutex);
			connect_set.erase(conn->GetFd());
			pthread_mutex_unlock(&_err_connect_mutex);
		//conn->LockSelf();
			conn->Close();	
			//conn->SetFd(INVALID_FD);	
		//}
		//conn->UnLockSelf();
	}
	*/
	
}
int ConnectionClient::create_pipe()
{
	int options;
	int pipe_fd[2];

	if (pipe(pipe_fd))
		return -1;

	_pipe_read = pipe_fd[0];
	_pipe_write = pipe_fd[1];

	for (int i=0; i<2; i++)
	{
		if ((options = fcntl(pipe_fd[i], F_GETFL)) == -1)
			return -1;
		if (fcntl(pipe_fd[i], F_SETFL, options | O_NONBLOCK) == -1)
			return -1;
	}

	return 0;
}
int ConnectionClient::Open(const std::string& conf,int receive_thread)
{
	
	if ((_epoll_fd = epoll_create(max_fd_num_)) == -1)
	{
		_err("epoll creat error\n");
		return -1;
	}
	
	if (create_pipe())
	{
		_err("create pipe error\n");
		return -1;
	}
	add_input_fd(_pipe_read);

	if (LoadConf(conf)<0){
		return -1;
	}

	//init check routine
	check_connect_inter(10); // every 10 s to check fd status	
	_check = new tcp_check(this);
	_check->Open(1);
	_check->Activate();
		
	STaskBase<Connection*>::Open(receive_thread);
	_debug("ConnectionClient::Open OK~\n"); 
	return 0;
}

int ConnectionClient::Svc()
{
	int fd;
	//char * buf = new char[MAX_BUFF_SIZE];
	while (1)
	{
		pthread_mutex_lock(&_epoll_mutex);
		update_fd();

		if (_stop_task) {
			pthread_mutex_unlock(&_epoll_mutex);
			break;
		}
		if (_epoll_ready_event_num <= 0)
			_epoll_ready_event_num = epoll_wait(_epoll_fd, _epoll_ready_event, MAX_EPOLL_EVENT_NUM, -1);

		if (_epoll_ready_event_num-- < 0)
		{
			if (errno == EINTR)
			{
				pthread_mutex_unlock(&_epoll_mutex);
				continue;
			}
			else
			{
				pthread_mutex_unlock(&_epoll_mutex);
				break;
			}
		}

		fd = _epoll_ready_event[_epoll_ready_event_num].data.fd;
//		if (_socket_handle[fd].flag & SYSTEM_PIPE)
		if(fd == _pipe_read)
		{
			pthread_mutex_unlock(&_epoll_mutex);
			continue;
		}

		del_input_fd(fd);
		pthread_mutex_unlock(&_epoll_mutex);

		//recv_result(fd,buf);
		//recv_routine(fd,buf);
		Connection * conn = GetConnection(fd);
		//impl_base_->Run((void*)conn);

		
		if (conn && impl_base_ && !(impl_base_->Run((void*)conn)<0)){
			//fprintf(stderr,"run ok\n");
			//conn->release();
			add_input_fd_record(fd);	
		}else {
			//fprintf(stderr,"run failed\n");
			if (conn){
          _err("Connection error~~\n");
				ErrorHandle(conn);
			}else {
			  _err("can not get the conn~~,close it\n");
			  close(fd);
			}
		}	

	}

	return 0;
}
Connection* ConnectionClient::GetConnection(int fd)
{	
	pthread_mutex_lock(&_err_connect_mutex);
	std::map<int, Connection*>::iterator it = connect_set.find(fd);
	if (it != connect_set.end()) 
	{
		//it->second->acquire();
		pthread_mutex_unlock(&_err_connect_mutex);
		return it->second;
	}
	pthread_mutex_unlock(&_err_connect_mutex);
	_err("ConnectionClient::GetConnection <btlswkxt> do not get connection !!\n");
	return NULL;
}

int ConnectionClient::add_input_fd(int fd)
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event);
	return 0;
}

int ConnectionClient::del_input_fd(int fd)
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &event);
	return 0;
}
int ConnectionClient::add_input_fd_record(int fd)
{
	char buf;
	pthread_mutex_lock(&_fds_update_mutex);

	_update_record[_update_record_num].op = EPOLL_CTL_ADD;
	_update_record[_update_record_num].fd = fd;
	_update_record[_update_record_num].event.events = EPOLLIN | EPOLLET;
	_update_record[_update_record_num].event.data.fd = fd;
	_update_record_num++;
	write(_pipe_write, &buf, 1);

	pthread_mutex_unlock(&_fds_update_mutex);
	return 0;
}

int ConnectionClient::update_fd()
{
	char buf[PIPE_BUF];
	pthread_mutex_lock(&_fds_update_mutex);

	for (int i=0; i<_update_record_num; i++)
		epoll_ctl(_epoll_fd, _update_record[i].op, _update_record[i].fd, &_update_record[i].event);

	_update_record_num = 0;
	while (read(_pipe_read, buf, PIPE_BUF) > 0);

	pthread_mutex_unlock(&_fds_update_mutex);
	return 0;
}


