#ifndef _SOGOU_MONITOR_H_
#define _SOGOU_MONITOR_H_

#include <utils/stask_base.h>
#include <map>
#include <list>
#include <vector>
#include <utils/scribe_proxy.h>
class TaskMonitorBase :public STaskBase<int>
{
public:
  TaskMonitorBase(){
	_bStop = true;
    	pthread_mutex_init(&mutex_, NULL);
  };
  ~TaskMonitorBase(){
    	pthread_mutex_destroy(&mutex_);
  };
  void SetInterval(int check_interval){interval_ =check_interval;};// s 
  typedef enum LogType 
  {
    AVG,
    SUM,
  };
  struct TaskInfo
  {
    unsigned long sum;
    unsigned long num;
    LogType type;    
  };
  int Start(){
    Open(1);
    Activate();
  }
  int Svc();

  virtual void CheckState(){};
  void AddType(const std::string& name,LogType type);
  void AddValue(const std::string& name,unsigned int value);
  TaskInfo GetInfo(const std::string& name);

protected:
  bool _bStop;
  std::string Map2String();
  std::map<std::string,struct TaskInfo> monitor_map_;
  pthread_mutex_t mutex_;
  int interval_;
  std::string category_;
  std::string id_;
};
class TaskMonitor :public TaskMonitorBase
{
public:
  TaskMonitor(std::string category,const std::string& ip,int port){
    client_.Init(ip,port,50,1);
    client_.Open();
    category_=category;
  }
  void SetUniqueId(std::string& id){
    id_=id;
  }
  ~TaskMonitor(){
    client_.Close();
  }
  virtual void CheckState();
    virtual int Close();
void AddString(std::string _category,std::string _value);

protected:
  ScribeProxyClient client_;
};

#endif
