#ifndef _DDB_CLINT_H_
#define _DDB_CLINT_H_

class DDBClient
{
public:
  int Open(const std::string& master_conf,int timeout);

  int Del(void* key,int length);
  int Get(void* key,int length,void*& value,int& value_len);
  int Set(void* key,int length,void* value,int value_len);
  
protected:
  int timeout_;
  DDBMasterHandler master_handler_;
  DDBLocateHandler locate_handler_;
  DDBDataHandler data_handler_;
};
#endif