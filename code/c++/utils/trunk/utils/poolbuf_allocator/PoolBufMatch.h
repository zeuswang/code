//多关键词匹配算法基类CMatch
//
//		$Revision: 1.2 $
//		$Author: wangying $
//		$Date: 2006/08/26 10:56:25 $
//
//////////////////////////////////////////////////////////////////////

#ifndef POOLBUF_MATCH_H_
#define POOLBUF_MATCH_H_

#include "poolbuf_allocator.hpp"

#define MAX_CHAR_SETS  256
#define MAX_PATTERN_LEN 20

//代理函数，再每次找到一个关键词模式的时候自动调用回调函数
//iPatternIndex: 匹配的关键词ID(从0到关键词个数-1)
//iFoundPos: 关键词匹配的偏移量(以字节为单位)。偏移量可能是匹配关键词的头部，也可能是尾部，视具体的实现算法
//pTag: 用户从search()传递过来的参数
//如果代理函数返回值<0，将终止后续文本扫描马上退出
typedef int (* FDelegate)(int iPatternIndex, int iFoundPos, void * pTag);


class CPoolBufMatch  
{
	
public:
	CPoolBufMatch();
	virtual ~CPoolBufMatch();

	//初始化函数
	//iNumber: 关键词个数
	//pLen[]: 关键词长度数组
	//pPatterns[]: 关键词指针数组
	//返回值小于0，初始化出错
	//初始化函数返回后，用户可以释放关键词数组所占的存储
	virtual int initialize(int iNumber, const int pLen[], const char * pPatterns[], ods_utils::PoolBufAlloc *alloc) = 0;

	//下面函数需要子类实现
	//在内存中扫描pBuff,iSize扫描关键词
	virtual int search(const char * pBuff, int iSize, FDelegate pCallBackReportFound, void * pTag) = 0;

	virtual void clear() = 0;

protected:

	//新开辟缓冲区，复制一份关键词数组文本
//	int copyPatterns(int iNumber, const int pLen[], const char * pPatterns[]);
//	void releaseCopy();

protected:
	//保存关键词文本的数组
	char ** m_ppPatterns;
	//保存每个关键词长度的数组
	int   * m_pPatLen;

	//由于关键词中可能包含二进制编码
	//所以不能使用strlen取关键词长度
	int   m_num;//关键词个数

};

#endif //POOLBUF_MATCH_H_
