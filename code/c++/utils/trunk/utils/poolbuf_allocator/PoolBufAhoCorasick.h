/********************************************************************
//		Aho_Corasick Ëã·¨
//		$Revision: 1.2 $
//		$Author: wangying $
//		$Date: 2006/08/26 10:56:25 $
********************************************************************/
#ifndef POOLBUF_AHOCORASICK_H_
#define POOLBUF_AHOCORASICK_H_
#include "poolbuf_allocator.hpp"

#include "PoolBufMatch.h"
#define CHARSIZE  MAX_CHAR_SETS
class CPoolBuf_Aho_Corasick :public CPoolBufMatch
{
public:
	CPoolBuf_Aho_Corasick();
	virtual ~CPoolBuf_Aho_Corasick();

	virtual int initialize(int iNumber, const int pLen[], const char * pPatterns[], ods_utils::PoolBufAlloc *alloc);
	virtual int search(const char * pBuff, int iSize, FDelegate pCallBackReportFound, void * pTag);
	virtual void clear();


private:
	typedef struct _stTrieNode
	{
		int * mMatchedPatterns;
		size_t mMatchedPatternsLen;	// number of ints in mMatchedPatterns allocated
		struct _stTrieNode* pstNext[CHARSIZE];
		struct _stTrieNode* pstFailure;
		struct _stTrieNode* pstChain;
	} stTrieNode;


	// global variabal
	stTrieNode* pstTrieRoot;
	stTrieNode* pstNodeChain;
};


#endif  //POOLBUF_AHOCORASICK_H_
