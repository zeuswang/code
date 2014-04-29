#include "ddb_client_protocol.h"

#define DBF_MEMMODE			03
#define DBF_USERMEM			00
#define DBF_PARTIAL			01
#define DBF_MALLOC			02
#define DBF_REALLOC			03
#define DBF_NOOVERWRITE		01
#define DBF_NOSYNC			01
#define DBF_NOP				04


/*struct __sohudb_net
{
	int sockfd;
	struct timeval *timeout;
};*/

struct sohudbt
{
	void *data;
	size_t size;
};

#ifndef __linux__
# define select(maxfdp1, rset, wset, xset, timeout) \
({																		\
	struct timeval __tv = *(const struct timeval *)(timeout);			\
	long __tps = sysconf(_SC_CLK_TCK);									\
	clock_t __clock = times(NULL);										\
	int __n = select(maxfdp1, rset, wset, xset, timeout);				\
	__clock = times(NULL) - __clock;									\
	*(timeout) = __tv;													\
	if (((timeout)->tv_sec -= __clock / __tps) < 0)						\
	{																	\
		(timeout)->tv_usec += 1000000 * (timeout)->tv_sec;				\
		(timeout)->tv_sec = 0;											\
	}																	\
	if (((timeout)->tv_usec -= __clock % __tps * 1000000 / __tps) < 0)	\
		(timeout)->tv_usec = 0;											\
	__n;																\
})
#endif

#define readn(x, y, z, timeout) \
({ \
	readn_timeout(x, y, z, timeout); \
})

inline static int wait_readable(int filedes, struct timeval *timeout)
{
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(filedes, &rset);
	return select(filedes + 1, &rset, NULL, NULL, timeout);
}

inline static ssize_t read_timeout(int filedes, void *buff, size_t nbytes,
								   struct timeval *timeout)
{
	int n = wait_readable(filedes, timeout);

	if (n > 0)
		return read(filedes, buff, nbytes);

	if (n == 0)
		errno = ETIME;

	return -1;
}

static ssize_t readn_timeout(int filedes, void *buff, size_t n,
							 struct timeval *timeout)
{
	size_t nleft = n;
	ssize_t nbytes;

	while (nleft > 0)
	{
		if ((nbytes = read_timeout(filedes, buff, nleft, timeout)) > 0)
		{
			buff = (char *)buff + nbytes;
			nleft -= nbytes;
		}
		else if (nbytes == 0)
			return n - nleft;
		else
			return -1;
	}

	return n;
}

inline static int extract_int(int fd, int *result, struct timeval *timeout)
{
	if (readn(fd, result, sizeof (int), timeout) == sizeof (int))
	{
		*result = ntohl(*result);
		return 0;
	}
	else
		return -1;
}

static int extract_dbt(int fd, int mode, struct sohudbt *dbt, struct timeval *timeout)
{
	unsigned int size;
	void *data;
	int ret = -1;

	if (extract_int(fd, (int *)&size, timeout) >= 0)
	{
		if (mode == DBF_REALLOC)
			data = dbt->data;
		else
			data = NULL;

		if (data = realloc(data, size))
		{
			if (readn(fd, data, size, timeout) == size)
			{
				switch (mode)
				{
				case DBF_MALLOC:
				case DBF_REALLOC:
					dbt->data = data;
					dbt->size = size;
					ret = 0;
					break;

				case DBF_USERMEM:
					if (dbt->size >= size)
					{
						memcpy(dbt->data, data, size);
						dbt->size = size;
						ret = 0;
					}
					else
						errno = ENOSPC;

					break;

				case DBF_PARTIAL:
					if (dbt->size < size)
						size = dbt->size;

					memcpy(dbt->data, data, size);
					ret = 0;
					break;

				default:
					errno = EINVAL;
				}
			}

			if (ret < 0 || mode != DBF_MALLOC && mode != DBF_REALLOC)
				free(data);
		}
	}

	return ret;
}

int DDBProtocolAdapter::GetSockAddr(int sockfd,
                                    void * key,
                                    uint32_t key_len,
                                    struct sockaddr *& serv_addr,
                                    int& addr_len,
                                    int flags,
                                    struct timeval* tv )
{
  uint8_t cmd = DDB_GET_LOCATION_CMD;
  uint32_t key_tmp=key_len;
  struct sohudbt data;
  struct iovec vector[] = {
    {
      iov_base  : &cmd,
      iov_len   : sizeof (unsigned uint8_t)
    },
    {
      iov_base  : &key_tmp,
      iov_len   : sizeof (uint32_t)
    },
    {
      iov_base  : key,
      iov_len   : key_len
    },
    {
      iov_base  : &flags,
      iov_len   : sizeof (int)
    }
  };
  int cnt = sizeof vector / sizeof vector[0];
  int mode = DBF_MALLOC & DBF_MEMMODE;
  int ret = -1;

  key_tmp = htonl(key_tmp);
  flags = htonl(flags);
  if (writev(sockfd, vector, cnt) >= 0)
  {
    if (extract_int(sockfd, &ret, tv) >= 0)
    {
      if (ret == 0)
      {
        if (extract_dbt(sockfd, mode, data, tv) < 0)
          ret = -1;
        else {
          serv_addr = data.data;
          addr_len =data.size;
        }
      }
      else if (ret < 0)
      {
        int error;
        if (extract_int(sockfd, &error, tv) < 0)
          ret = -1;
        else
          errno = error;
      }
    }
  }

  return ret;
}
int DDBProtocolAdapter::Del(int sockfd,
                            void* key,
                            int key_len,
                            int flags,
                            struct timeval* tv)

{
  uint8_t cmd = DDB_CLIENT_DEL_CMD;
  uint32_t key_tmp=key_len;
  struct sohudbt data;
  struct iovec vector[] = {
    {
      iov_base  : &cmd,
      iov_len   : sizeof (unsigned uint8_t)
    },
    {
      iov_base  : &key_tmp,
      iov_len   : sizeof (uint32_t)
    },
    {
      iov_base  : key,
      iov_len   : key_len
    },
    {
      iov_base  : &flags,
      iov_len   : sizeof (int)
    }
  };
  int cnt = sizeof vector / sizeof vector[0];
  int mode = DBF_MALLOC & DBF_MEMMODE;
  int ret = -1;

  key_tmp = htonl(key_tmp);
  flags = htonl(flags);
	if (writev(sockfd, vector, cnt) >= 0)
	{
		if (extract_int(sockfd, &ret, tv) >= 0)
		{
			if (ret < 0)
			{
				int error;
				if (extract_int(sockfd, &error, tv) < 0)
					ret = -1;
				else
					errno = error;
					//errno = ntohl(error);
			}
		}
	}


  return ret;
}

int DDBProtocolAdapter::Get(int sockfd,
                            void* key,
                            int key_len,
                            void*& value,
                            int& value_len,
                            int flags,
                            struct timeval* tv)

{
  uint8_t cmd = DDB_CLIENT_GET_CMD;
  uint32_t key_tmp=key_len;
  struct sohudbt data;
  struct iovec vector[] = {
    {
      iov_base  : &cmd,
      iov_len   : sizeof (unsigned uint8_t)
    },
    {
      iov_base  : &key_tmp,
      iov_len   : sizeof (uint32_t)
    },
    {
      iov_base  : key,
      iov_len   : key_len
    },
    {
      iov_base  : &flags,
      iov_len   : sizeof (int)
    }
  };
  int cnt = sizeof vector / sizeof vector[0];
  int mode = DBF_MALLOC & DBF_MEMMODE;
  int ret = -1;

  key_tmp = htonl(key_tmp);
  flags = htonl(flags);
  if (writev(sockfd, vector, cnt) >= 0)
  {
    if (extract_int(sockfd, &ret, tv) >= 0)
    {
      if (ret == 0)
      {
        if (extract_dbt(sockfd, mode, data, tv) < 0)
          ret = -1;
        else {
          value = data.data;
          value_len =data.size;
        }
      }
      else if (ret < 0)
      {
        int error;
        if (extract_int(sockfd, &error, tv) < 0)
          ret = -1;
        else
          errno = error;
      }
    }
  }

  return ret;
}

int DDBProtocolAdapter::Set(int sockfd,
                            void* key,
                            int key_len,
                            void* value,
                            int value_len,
                            int flags,
                            short digest,
                            struct timeval* tv)

{
  uint8_t cmd = DDB_CLIENT_SET_CMD;
	unsigned int key_len_tmp = key_len;
	unsigned int data_len_tmp = value_len;
	struct iovec vector[] = {
		{
			iov_base	:	&cmd,
			iov_len		:	sizeof (uint8_t)
		},
		{
			iov_base	:	&key_len,
			iov_len		:	sizeof (uint32_t)
		},
		{
			iov_base	:	key,
			iov_len		:	key_len
		},
		{
			iov_base	:	&data_len_tmp,
			iov_len		:	sizeof (uint32_t)
		},
		{
			iov_base	:	value,
			iov_len		:	value_len
		},
		{
			iov_base	:	&digest,
			iov_len		:	sizeof (short)
		},
		{
			iov_base	:	&flags,
			iov_len		:	sizeof (int)
		}
	};
	int cnt = sizeof vector / sizeof vector[0];
	int ret = -1;

	key_len_tmp = htonl(key_len);
	data_len_tmp = htonl(data_len_tmp);
	digest = htons(digest);
	flags = htonl(flags);
	if (writev(sockfd, vector, cnt) >= 0)
	{
		if (extract_int(sockfd, &ret, tv) >= 0)
		{
			if (ret < 0)
			{
				int error;
				if (extract_int(sockfd, &error, tv) < 0)
				{
					ret = -1;
				}
				else
				{
					errno = error;
					//errno = ntohl(error);
				}
			}
		}
	}

	return ret;

}
/*
int MakeRequest(uint8_t cmd,void* key,uint32_t len,void* value,uint32_t v_len,uint8_t flags)
{
  unsigned int key_len = len;
	struct iovec vector[] = {
		{
			iov_base	:	&cmd,
			iov_len		:	sizeof (uint8_t)
		},
		{
			iov_base	:	&key_len,
			iov_len		:	sizeof (uint32_t)
		},
		{
			iov_base	:	key,
			iov_len		:	key_len
		},
		{
			iov_base	:	&data_len,
			iov_len		:	sizeof (unsigned int)
		},
		{
			iov_base	:	data->data,
			iov_len		:	data->size
		},
		{
			iov_base	:	&digest,
			iov_len		:	sizeof (short)
		},
		{
			iov_base	:	&flags,
			iov_len		:	sizeof (int)
		}
	};

  int cnt = sizeof vector / sizeof vector[0];
  int mode = flags & DBF_MEMMODE;
  int ret = -1;
  
  key_len = htonl(key_len);
  flags = htonl(flags);
  if (writev(db->sockfd, vector, cnt) >= 0)
  {
    if (extract_int(db->sockfd, &ret, timeout) >= 0)
    {
      if (ret == 0)
      {
        if (extract_dbt(db->sockfd, mode, data, timeout) < 0)
          ret = -1;
      }
      else if (ret < 0)
      {
        int error;
        if (extract_int(db->sockfd, &error, timeout) < 0)
          ret = -1;
        else
          errno = error;
      }
    }
  }
  
  return ret;


}
int MakeResult(uint8_t ret,int error,void* value,uint32_t v_len)
{

}
*/
