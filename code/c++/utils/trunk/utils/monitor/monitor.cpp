#include "monitor.h"
#include "utils/utils/utils.h"
int TaskMonitorBase::Svc()
{
  while(_bStop){
    CheckState();
    sleep(interval_);
  }
}
void TaskMonitorBase::AddType(const std::string& name,LogType type)
{
  std::map<std::string,TaskInfo>::iterator it;
  pthread_mutex_lock(&mutex_);
  it=monitor_map_.find(name);
  if (it!=monitor_map_.end()){
    it->second.type=type;
  }else {
    TaskInfo info;
    info.num=0;
    info.sum=0;
    info.type=type;
    monitor_map_.insert(std::map<std::string,TaskInfo>::value_type(name,info));
  }
  pthread_mutex_unlock(&mutex_);
  return ;
}
void TaskMonitorBase::AddValue(const std::string& name,unsigned int value)
{

  std::map<std::string,TaskInfo>::iterator it;
	pthread_mutex_lock(&mutex_);
  it=monitor_map_.find(name);
  if (it!=monitor_map_.end()){
    it->second.sum +=value;
    it->second.num +=1;
  }
 	pthread_mutex_unlock(&mutex_);
}
TaskMonitorBase::TaskInfo TaskMonitorBase::GetInfo(const std::string& name)
{
	std::map<std::string,TaskInfo>::iterator it;
	pthread_mutex_lock(&mutex_);
	it=monitor_map_.find(name);
	if (it!=monitor_map_.end())
	{
		pthread_mutex_unlock(&mutex_);
		return it->second;
	}
	else 
	{
		pthread_mutex_unlock(&mutex_);
		TaskInfo info;
		return info;
	}
}
std::string TaskMonitorBase::Map2String()
{
  std::map<std::string,TaskInfo>::iterator it;
  //init log 
  std::string log;
  char tmp[128];
  std::string id = sogou_utils::GetLocalIp();
  if(!id_.empty())
  	id = sogou_utils::GetLocalIp()+":"+id_;
  
  snprintf(tmp,128,"%s\t%s\t%d\t",id.c_str(),category_.c_str(),time(NULL));
  log.append(tmp);

  	pthread_mutex_lock(&mutex_);
	for(it = monitor_map_.begin(); it != monitor_map_.end(); ++it) {
    char buf[64];
    if (it->second.type == SUM){
      snprintf(buf,64,"%d",it->second.sum);
      log += it->first + ":" +std::string(buf)+",";
    }else if (it->second.type == AVG){
      float value=0;
      if (it->second.num >0){ 
        fprintf(stderr,"type=%s,sum=%d,num=%d\n",it->first.c_str(),it->second.sum,it->second.num);
        value = (it->second.sum / (float)it->second.num );
      }
      snprintf(buf,64,"%.02f",value);
      log += it->first + ":" +std::string(buf)+",";
    }
    it->second.sum=0;
    it->second.num=0;
	}
  pthread_mutex_unlock(&mutex_);
  log +="Owner=OP\n";
  return log;
}
void TaskMonitor::CheckState()
{
  std::vector<std::string> list;
  list.push_back(category_);
  client_.Send(list,Map2String());
}


int TaskMonitor::Close()
{
	_bStop =false;  
	STaskBase<int>::Close(); 
}



void TaskMonitor::AddString(std::string _category,std::string _value)
{
	std::vector<std::string> list;
	list.push_back(_category);
	_value.append("\n");
	fprintf(stderr,"[ScribeInfo]%s_%s\n",_category.c_str(),_value.c_str());
	client_.Send(list,_value);
}

