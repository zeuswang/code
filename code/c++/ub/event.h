#ifndef _UB_EVENT_H_
#define _UB_EVENT_H_
namespace ub
{
#define FOR_HEAD 0
#define FOR_BODY 1 
static const int g_SMALLBUFSIZE = 256;

typedef struct _ub_socket_t {		  /**<    socket�������    */
	UbEvent * event;		  /**<   event     */
	IReactor * first_reactor;		  /**<   reactor     */
	unsigned int max_buffer_size;		  /**<   ����ڴ�     */
	unsigned int max_readbuf_size;		  /**<   �����ڴ�     */
	unsigned int max_writebuf_size;		  /**<   ���д�ڴ�     */
	char small_readbuf[g_SMALLBUFSIZE];	  /**<   ����С����ʱʹ�õĻ�����     */
	char small_writebuf[g_SMALLBUFSIZE];		  /**<   ��дС����ʱʹ�õĻ�����     */
	char * read_buf;		  /**<   ������ָ��     */
	int read_timeout;		  /**<   ����ʱms     */
	unsigned int read_buf_len;     /**<   ���������ܳ���     */
	unsigned int read_buf_used;    /**<   �Ѷ������ݳ���     */
	char * write_buf;		  /**<  д����ָ��      */
	int write_timeout;		  /**<  д��ʱms      */
	unsigned int write_buf_len;     /**<   д�������ܳ���     */
	unsigned int write_buf_used;     /**<   ��д�����ݳ���     */
//	struct {
		int ext_readbuf; /**<   ʹ���ⲿ������     */
		int ext_writebuf; /**<   ʹ���ⲿд����     */
//	};


	int connect_type;		  /**<   �������ͣ���������     */
	int connect_timeout;   /**<  ���ӳ�ʱms      */
	int process_timeout;   /**<  �������Ժ��session����ʱms      */
	ub_timecount_t timer;		  /**<   ��ʱ��     */
	struct sockaddr_in client_addr;		  /**<   ���ӵ�ַ     */
	unsigned int status;		  /**<   event״̬     */
	int sock_opt;	  /**<   socketѡ��     */
} ub_socket_t; /**<    socket�������    */

class UbEvent :public IEvent
{
public:
    	enum {
		STATUS_READ = 0x0A,		      /**<  ������״̬  */
		STATUS_WRITE,		          /**<  д����״̬  */
		STATUS_ACCEPTED,		      /**<  accept���״̬  */
		STATUS_ERROR = 0x100,		  /**<  ����  */
		STATUS_TIMEOUT = 0x200,		  /**<  ��ʱ  */
		STATUS_CANCEL = 0x400,		  /**<  ȡ��  */
		STATUS_CLOSESOCK = 0x800,	  /**<  �Զ˹ر�����  */
		STATUS_POSTERROR = 0x1000,	  /**<  �����¼�ʧ��  */
		STATUS_MEMERROR = 0x2000,	  /**<  �ڴ����ʧ��  */
		STATUS_PRESESSIONERROR = 0x4000,  /**<  �ڴ����ʧ��  */
		STATUS_USERDEF = 0x1000000,		  /**<  �Զ���״̬����Сֵ  */
		STATUS_CONNERROR = 0x2000000              /**<  ����ʧ��     */
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


