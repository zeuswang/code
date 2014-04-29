#ifndef _SOGOU_TOPOLOGY_H_
#define _SOGOU_TOPOLOGY_H_
#include <string>
#include <vector>
#include "sconnection.h"
class Connection;
typedef enum 
{
	T_Root,
	T_Hash,
	T_Circle,
	T_DocidCircle,
	T_Mirror,
	T_DHT,
	T_Node
}TopologyType;


class VNode
{
friend class NodeTopology;
public:
	VNode():addr_(""),port_(""),index_(0),busy_(false){
		pthread_mutex_init(&mutex_, NULL);
		pthread_mutex_init(&flag_mutex_, NULL);
	};
	~VNode(){Close();};
	std::string GetAddress(){return addr_;};
	std::string GetPort(){return port_;};
	std::vector<Connection*>& GetList(){return conn_list_;};
	
	void SetAddress(const std::string& ip,const std::string& port){
		addr_ = ip;
		port_ = port;
	};
	void LockSelf(){pthread_mutex_lock(&mutex_);};
	void UnLockSelf(){pthread_mutex_unlock(&mutex_);};
	void SetBusy(){
		pthread_mutex_lock(&flag_mutex_);
		busy_=true;
		pthread_mutex_unlock(&flag_mutex_);
	}
	void SetUnBusy(){
		pthread_mutex_lock(&flag_mutex_);
		busy_=false;
		pthread_mutex_unlock(&flag_mutex_);
	}
	bool Busy(){
		bool busy=false;
		pthread_mutex_lock(&flag_mutex_);
		busy=busy_;
		pthread_mutex_unlock(&flag_mutex_);
		return busy;
	}
	Connection* CreateNewConnection(){
		Connection* conn= new Connection();
		//conn->Connect(addr_,port_);		
		conn_list_.push_back(conn);
		return conn;
	};
	
	Connection* SelectConnection(const std::string& key){
		if (conn_list_.size()== 0)
			return NULL;
		for (size_t i=0;i<conn_list_.size();i++){
			index_ = (index_ +1) % conn_list_.size();		
			//if ((conn_list_[index_])&&(conn_list_[index_]->StateOk())&&(conn_list_[index_]->ref()==0))
			if ((conn_list_[index_])&&(conn_list_[index_]->ref()==0))
				return conn_list_[index_];
		}
		return NULL;			
	};
	void Close(){
		for (size_t i=0;i<conn_list_.size();i++){
			if (conn_list_[i])
				delete conn_list_[i];
		}
		pthread_mutex_destroy(&mutex_);
		pthread_mutex_destroy(&flag_mutex_);
	};
protected:
	std::string addr_;
	std::string port_;
	int index_;
	bool busy_;
	std::vector<Connection*> conn_list_;
	pthread_mutex_t flag_mutex_;
	pthread_mutex_t mutex_;


};


class Topology
{
public:
	Topology(){};
	virtual ~Topology(){Close();};
	virtual Connection* SelectConnection(const std::string&key){return NULL;};
  	virtual Connection* SelectConnection(void* key,int len){return NULL};
  virtual class NodeTopology* Select(const std::string& key){return NULL};
  virtual class NodeTopology* Select(const std::string& key,int len){return NULL};
	//virtual int LoadConf(const std::string& conf);
	virtual TopologyType GetType()=0;
	virtual VNode* GetVNode(){return NULL;};
	virtual void AddChild(Topology * topology){tlist_.push_back(topology);};
	std::vector<Topology*>& GetList(){return tlist_;};
	virtual void Close(){
		for (size_t i=0;i<tlist_.size();i++){
			delete tlist_[i];
		}
	};
protected:
	std::vector<Topology*> tlist_;
};

class HashCircleTopology:public Topology
{
public:
	HashCircleTopology(){};
	virtual ~HashCircleTopology(){};
	TopologyType GetType(){return T_Hash;};
	virtual Connection* SelectConnection(const std::string&key){return NULL};
	virtual NodeTopology* Select(const std::string&key);
  	virtual NodeTopology* Select(const std::string&key,int len);
protected:
	TopologyType type_;
	int index_;
};

class DocidCircleTopology:public Topology
{
public:
	DocidCircleTopology(){};
	virtual ~DocidCircleTopology(){};
	TopologyType GetType(){return T_Circle;};
	virtual Connection* SelectConnection(const std::string&key);
	virtual class NodeTopology* Select(const std::string& key){return NULL;};
	
protected:
	TopologyType type_;
	int index_;
};

class NodeTopology:public Topology
{
public:
	NodeTopology(){};
	virtual ~NodeTopology(){};
	TopologyType GetType(){return T_Node;};
	bool Busy(){
		return node_.Busy();
	};
	virtual Connection* SelectConnection(const std::string&key);
	virtual NodeTopology* Select(const std::string& key){
		if (Busy()) 
			return NULL;
		else 
			return this;
	};
	virtual VNode* SelectVNode(const std::string&key){return &node_;};
	virtual VNode* GetVNode(){return &node_;};
	
protected:
	TopologyType type_;
	VNode node_;
};

class TopologyFactory
{
public:
	TopologyFactory(){

	};
	~TopologyFactory(){

	};
	Topology* GetNewTopology(const std::string& conf);
};


#if 0
class DhtTopology:public Topology
{
public:
	DhtTopology(){
		max_num_=256;
		table_ok=false;
		num_node_[0].first=0;
		num_node_[0].second=256;
		memset(num_node_,0,256);
		memset(table_,0,256);
	};
	virtual ~DhtTopology(){};
	TopologyType GetType(){return T_DHT;};
	virtual NodeTopology* Select(const std::string&key);
protected:
	unsigned char FindMaxHandlerIndex(){
		int max =0;
		unsigned char index=0;
		for (unsigned char i=0;i<255;i++)
		{
			if (num_node_[i].second - num_node_[i].first) >max){
				max = num_node_[i].second - num_node_[i].first;
				i=index;
			}				
		}
		return index;
	}
	int max_num_;
	unsigned char[256] table_;
	std::pair<int,int> [256] num_node_;
	bool table_ok;
	
	
};
NodeTopology* DhtTopology::Select(const std::string&key)
{
	if (table_ok){
		unsigned char index = key[0] % 255;
		return tlist_[index].Select(key.substr(1,key.length()-1);
	}
	else{
		//init the hash table 
		for (unsigned char i=1;i<tlist_.size()-1;i++)
		{
			unsigned char index = FindMaxHandlerIndex();
			if (num_node_[index].first < num_node_[index].first)
			{
				num_node_[i].first = num_node_[index].first+((num_node_[index].second - num_node_[index].first)/2+1);
				num_node_[i].second = num_node_[index].second;
				for (unsigned char j=num_node_[i].first;j<=num_node_[i].second;j++){
					table_[j]=i;
				}
			}			
		}			
	}
}


#endif

#endif 
