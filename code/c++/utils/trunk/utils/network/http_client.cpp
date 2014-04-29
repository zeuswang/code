#include "http_client.h"

int HttpClient::Get(HttpProtocolAdapter* adapter,const std::string& key)
{
	Connection * conn = GetConnection(key);
	if (!conn)
		return -1;
	adapter->SetConnection(conn);
	if (adapter.PutRequest()<0){
		//conn->LockSelf();
		//conn->Reconnect();
		ReleaseConnection(conn);
		//conn->UnLockSelf();
		return -1;
	}
	if (adapter.GetResult()<0){
		//conn->LockSelf();
		//conn->Reconnect();
		ReleaseConnection(conn);
		//conn->UnLockSelf();
		return -1;
	}
	ReleaseConnection(conn);
	return 0;
}

int HttpClient::CheckConnection(Connection* conn)
{
	/*if (conn){
		if conn->
		conn->LockSelf();
		int cost_avg = (conn->cost_info_.cost_sum)/check_interval_;
		if (cost_avg > (0.8* timeout_))
			conn->GetVNode()->SetBusy();
		conn->cost_info_.cost_sum=0;
		conn->UnLockSelf();
	}*/
}





