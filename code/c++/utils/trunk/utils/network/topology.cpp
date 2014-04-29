#include "topology.h"
#include <Platform/docId/docId.h>
#include <utils/sxml.h>
#include "offdb/OffDB.h"
#include "offdb/OffPage.h"
static int Node2Topology(SXmlNode node,Topology* topology)
{
	for (node;!node.Null();node=node.GetNext())
	{
		if (node.GetName()!="node")
			continue;
		std::string type = node.GetAttribute("type");		
		if (type==""){
			int conn_num= atoi(node.GetAttribute("conn_num").c_str());
			std::string addr = node.GetContent();
			size_t pos = addr.find(":");
			std::string ip,port;
			if (pos!=std::string::npos){
				ip = addr.substr(0,pos);
				port = addr.substr(pos+1,addr.length()-pos-1);
			}
			NodeTopology* node_topology=new NodeTopology();
			node_topology->GetVNode()->SetAddress(ip,port); 
			for (int i=0;i<conn_num;i++){
				Connection* conn=node_topology->GetVNode()->CreateNewConnection();
				conn->Connect(ip,port);
				conn->SetVNode(node_topology->GetVNode());
			}
			topology->AddChild(node_topology);
		}else if (type =="qdb"){

			int conn_num= atoi(node.GetAttribute("conn_num").c_str());
			std::string addr = node.GetContent();
			size_t pos = addr.find(":");
			std::string ip,port;
			if (pos!=std::string::npos){
				ip = addr.substr(0,pos);
				port = addr.substr(pos+1,addr.length()-pos-1);
			}
			NodeTopology* node_topology=new NodeTopology();
			node_topology->GetVNode()->SetAddress(ip,port); 
			for (int i=0;i<conn_num;i++){
				Connection* conn=node_topology->GetVNode()->CreateNewConnection();
				OffDB* db= new OffDB();
				fprintf(stderr,"qdb ip=%s,db=%p",addr.c_str(),db);
				//db->open(addr.c_str());
				conn->private_data = (void * )db;
				conn->SetVNode(node_topology->GetVNode());
			}
			topology->AddChild(node_topology);

		}else if (type=="hash_topology"){
			HashCircleTopology* dht = new HashCircleTopology();
			topology->AddChild(dht);
			if (Node2Topology(node.GetChildren(),dht))
				return -1;
		}else if (type=="docid_topology"){
			DocidCircleTopology* dht = new DocidCircleTopology();
			topology->AddChild(dht);
			if (Node2Topology(node.GetChildren(),dht))
				return -1;
		}
	}
	return 0;
}
Topology* TopologyFactory::GetNewTopology(const std::string& conf)
{
	SXmlDocument doc;	
	if (doc.Parse(conf,"utf-8"))
		return NULL;
	SXmlNode node = doc.GetRoot();
	Topology* t=NULL;
	if (node.GetName()=="hash_topology")
		t= new HashCircleTopology();
	else if (node.GetName()=="docid_topology")
		t =new DocidCircleTopology();
	
	if (t){
		 Node2Topology(node.GetChildren(),t);
		 return t;
	}else 
		return NULL;
}

Connection* DocidCircleTopology::SelectConnection(const std::string&key)
{

#if 1
	int client_size = tlist_.size();
	gDocID256_t doc_256;
	make_docid256(&doc_256,key.c_str());
	int num = get_docid_cycle_by_number(doc_256, client_size);
	return tlist_[num]->SelectConnection(key);
#endif
#if 0
	unsigned long long high,low;
	sscanf(key.c_str(),"%llx-%llx",&high,&low);
	char id[RESULT_SET_KEY_LENGTH];
	memcpy(id,&low,sizeof(low));
	memcpy(id+sizeof(low),&high,sizeof(high));
	int client_size = tlist_.size();
	gDocID_t docid = *(gDocID_t*)id;
	unsigned long long val = docid.id.value.value_high>>32;
	//WEBSEARCH_DEBUG((LM_DEBUG, "circle is %d \n ",(val*client_size)>>32));
	int num = (val*client_size)>>32;
	return tlist_[num]->SelectConnection(key);
/*		
	gDocID256_t* doc_256;
	make_docid256(doc_256,key.c_str());
	gDocID256_t* docid = (gDocID256_t*)doc_id;
	int num = get_docid_cycle_by_number(*docid, t_list_.size());
	return tlist_[num].SelectConnection(key);
*/
#endif
}
Connection* NodeTopology::SelectConnection(const std::string&key)
{
	return node_.SelectConnection(key);
}
NodeTopology* HashCircleTopology::Select(const std::string&key)
{
	int high2=0;
	{	
	int t1= key[0]>='a'?(key[0]-'a'+10):(key[0]-'0');
	//fprintf(stderr,"t1=%d,worker->qsid[0]=%x\n",t1,worker->qsid[0]);
	int t2=key[1]>='a'?(key[1]-'a'+10):(key[1]-'0');
	//fprintf(stderr,"t2=%d,worker->qsid[1]=%x\n",t2,worker->qsid[1]);
	high2=t1*16+t2;	
	}
	//WEBSEARCH_DEBUG((LM_DEBUG,"GetCircleNo qsid =%016llx\n",qsid));
	//WEBSEARCH_DEBUG((LM_DEBUG,"GetCircleNo qsid high 2bit=%llu\n",high2));
	//worker->circle =high2;

	int index =  high2 % tlist_.size(); 
	return tlist_[index]->Select(key);
}
NodeTopology* HashCircleTopology::Select(const std::string&key,int len)
{
	int high2=0;
  for(int i=0;i<len;i++)
  {
    	int t1= key[i]>='a'?(key[i]-'a'+10):(key[i]-'0');
     high2 =high2*16+ t1;

  }
	int index =  high2 % tlist_.size(); 
	return tlist_[index]->Select(key);
}




