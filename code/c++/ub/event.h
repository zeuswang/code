#ifndef _UB_EVENT_H_
#define _UB_EVENT_H_
namespace ub
{
#define FOR_HEAD 0
#define FOR_BODY 1 
static const int g_SMALLBUFSIZE = 256;

typedef struct _ub_socket_t {		  /**<    socket相关数据    */
	UbEvent * event;		  /**<   event     */
	IReactor * first_reactor;		  /**<   reactor     */
	unsigned int max_buffer_size;		  /**<   最大内存     */
	unsigned int max_readbuf_size;		  /**<   最大读内存     */
	unsigned int max_writebuf_size;		  /**<   最大写内存     */
	char small_readbuf[g_SMALLBUFSIZE];	  /**<   当读小数据时使用的缓冲区     */
	char small_writebuf[g_SMALLBUFSIZE];		  /**<   当写小数据时使用的缓冲区     */
	char * read_buf;		  /**<   读缓冲指针     */
	int read_timeout;		  /**<   读超时ms     */
	unsigned int read_buf_len;     /**<   读缓冲区总长度     */
	unsigned int read_buf_used;    /**<   已读的数据长度     */
	char * write_buf;		  /**<  写缓冲指针      */
	int write_timeout;		  /**<  写超时ms      */
	unsigned int write_buf_len;     /**<   写缓冲区总长度     */
	unsigned int write_buf_used;     /**<   已写的数据长度     */
//	struct {
		int ext_readbuf; /**<   使用外部读缓冲     */
		int ext_writebuf; /**<   使用外部写缓冲     */
//	};


	int connect_type;		  /**<   连接类型：长短连接     */
	int connect_timeout;   /**<  连接超时ms      */
	int process_timeout;   /**<  连接上以后的session处理超时ms      */
	ub_timecount_t timer;		  /**<   计时器     */
	struct sockaddr_in client_addr;		  /**<   连接地址     */
	unsigned int status;		  /**<   event状态     */
	int sock_opt;	  /**<   socket选项     */
} ub_socket_t; /**<    socket相关数据    */

class UbEvent :public IEvent
{
public:
    	enum {
		STATUS_READ = 0x0A,		      /**<  读数据状态  */
		STATUS_WRITE,		          /**<  写数据状态  */
		STATUS_ACCEPTED,		      /**<  accept句柄状态  */
		STATUS_ERROR = 0x100,		  /**<  出错  */
		STATUS_TIMEOUT = 0x200,		  /**<  超时  */
		STATUS_CANCEL = 0x400,		  /**<  取消  */
		STATUS_CLOSESOCK = 0x800,	  /**<  对端关闭连接  */
		STATUS_POSTERROR = 0x1000,	  /**<  抛起事件失败  */
		STATUS_MEMERROR = 0x2000,	  /**<  内存分配失败  */
		STATUS_PRESESSIONERROR = 0x4000,  /**<  内存分配失败  */
		STATUS_USERDEF = 0x1000000,		  /**<  自定义状态的最小值  */
		STATUS_CONNERROR = 0x2000000              /**<  连接失败     */
	};
    UbEvent():_fheader_length(0), _fbody_length(0), _fbody_readdone(0),_io_status(0){
        _ref=0;
        sock_data.event = this;
        sock_data.read_buf = sock_data.small_readbuf;
        sock_data.read_buf_len = sizeof(sock_data.small_readbuf);
        sock_data.read_buf_used = 0;
        sock_data.write_buf = sock_data.small_writebuf;
        sock_data.write_buf_len = sizeof(sock_data.small_writebuf);
        sock_data.write_buf_used = 0;
        sock_data.max_writebuf_size = 10*1024*1024;
        sock_data.max_readbuf_size = 10*1024*1024;
        //sock_data.sock_opt = 0;
        //sock_data.status = 0;
        //mgr_ =NULL;
        pthread_mutex_init(&_mutex);
        gettimeofday(&_start_time, NULL);

    };
    ~UbEvent();

public:
    int post();
    char* get_write_buffer(unsigned int size);
    void event_error_callback();
    void callback();

public:
    // need to inherit by subclass
    virtual int check_header(char * buf,int  len){return 0;};

    virtual int read_done(char * buf,int len){return 0;};

    virtual int write_done(){return 0;};
    virtual void error_handle();
    bool need_continue();

public:
    
    int _fheader_length;   /**<  header length      */
    int _fbody_length;		  /**<  http body length(Content-Length)      */
    int _fbody_readdone;		  /**<   http body read     */


    virtual int handle() { return _handle; }
    virtual void setHandle(int hd){_handle = hd;};
    virtual int events() { return _events; }
    virtual void setEvents(int ev) { _events = ev; }
    virtual int status () { return _status;}
    virtual void setStatus(int s) { _status = s; }
    virtual int type() { return _type; }
    virtual void setType(int t) { _type = t; }

    virtual bool release();
    
    virtual int clear() { return 0; }
    virtual IEvent * next() { return _next; }
    virtual void setNext(IEvent *ev) { _next = ev; }
    virtual IEvent *previous() { return _pre; }
    virtual void setPrevious(IEvent *ev) { _pre = ev; }
    
    virtual bool isTimeout() ;
    virtual void setTimeout_ms(int tv);

    IReactor *reactor() { return _reactor; }
    void setReactor(IReactor *r) { _reactor = r; }

    int addRef();
    int delRef();

protected:

    struct timeval  _start_time;
    int _tv;
    
    pthread_mutex_t _mutex;
    int _ref;
    int _handle;
    int _type;
    int _events;
    int _status;

    IReactor *_reactor;
    
    IEvent *_pre;
    IEvent *_next;

    
    /*virtual void setCallback(cb_t cb, void *p){
        callback_ = cb;
        user_params_ = p;

    };*/
    int read_buffer_process(int len);
    void * ub_event_malloc(size_t size);
    void ub_event_free(void * mem,size_t msize);
    void release_write_buf();
    void release_read_buf();
    const char * get_read_buffer(unsigned int size, int copy);

    
    ub_socket_t sock_data;
    int _io_status;
    void * user_params_;
    cb_t  callback_;
    //bsl::mempool * mgr_;
};

};



#endif 


