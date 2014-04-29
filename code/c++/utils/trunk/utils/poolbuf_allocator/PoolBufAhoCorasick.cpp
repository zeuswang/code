#include "PoolBufAhoCorasick.h"
#include <string.h>

//#include <cstdlib>
#include <queue>

using namespace ods_utils;

CPoolBuf_Aho_Corasick::CPoolBuf_Aho_Corasick()
{
	pstNodeChain = NULL;
	pstTrieRoot = NULL;
}

CPoolBuf_Aho_Corasick::~CPoolBuf_Aho_Corasick()
{
}

void CPoolBuf_Aho_Corasick::clear()
{
	stTrieNode * pstTmpPt;

	// no need to delete poolbufed items
//  while(pstNodeChain!=NULL)
//  {
//  	pstTmpPt = pstNodeChain;
//  	pstNodeChain = pstNodeChain->pstChain;
//  	delete [](pstTmpPt->mMatchedPatterns);
//  	delete pstTmpPt;
//  }
	pstNodeChain = NULL;
	pstTrieRoot = NULL;
}

int CPoolBuf_Aho_Corasick::initialize(int iNumber, const int pLen[], const char * pPatterns[], ods_utils::PoolBufAlloc *alloc)
{
	int iPat;

	pstTrieRoot = PoolBufAllocAdapter<stTrieNode>(alloc).NewObj();
	if(pstTrieRoot==NULL) return -1;  // not enough memory
	memset(pstTrieRoot,0,sizeof(stTrieNode));
	pstTrieRoot->pstChain = pstNodeChain;
	pstNodeChain = pstTrieRoot;
	
	size_t newed = 0, deleted = 0, poolbufed = 0;	// for memory debug

	// build basic trie
	for(iPat=0; iPat<iNumber; iPat++)
	{
		int len = pLen[iPat];
		int i;
		stTrieNode * pstNode;
		pstNode = pstTrieRoot;

		for(i=0; i< len; i++)
		{
			if(pstNode->pstNext[(unsigned char)pPatterns[iPat][i]]==NULL)
			{
				stTrieNode* pstNewNode;
				pstNewNode = PoolBufAllocAdapter<stTrieNode>(alloc).NewObj();
				if(pstNewNode==NULL) return -1;
				memset(pstNewNode,0,sizeof(stTrieNode));
				pstNewNode->pstChain= pstNodeChain;
				pstNodeChain = pstNewNode;
				pstNode->pstNext[(unsigned char)pPatterns[iPat][i]] = pstNewNode;
			}
			pstNode = pstNode->pstNext[(unsigned char)pPatterns[iPat][i]];			
		}
		if (pstNode->mMatchedPatterns == NULL)
		{
			pstNode->mMatchedPatterns = new int[2];	// 这里new的可能是临时的。全部完成后再一并转成PoolBufAlloc
			newed += sizeof(int) * 2;
			pstNode->mMatchedPatterns[0] = iPat;
			pstNode->mMatchedPatterns[1] = -1;
			pstNode->mMatchedPatternsLen = 2;
		}
		else
		{
			int mlen = 0;
			int * pm = pstNode->mMatchedPatterns;
			while (*pm != -1)
			{
				mlen ++;
				pm ++;
			}
			int * pnewMatched = new int[mlen + 2];	// 这里new的可能是临时的。全部完成后再一并转成PoolBufAlloc
			newed += sizeof(int) * (mlen + 2);
			memcpy(pnewMatched, pstNode->mMatchedPatterns, mlen*sizeof(int));
			pnewMatched[mlen] = iPat;
			pnewMatched[mlen + 1] = -1;

			delete [](pstNode->mMatchedPatterns);
			deleted += pstNode->mMatchedPatternsLen * sizeof(int);
			pstNode->mMatchedPatterns = pnewMatched;
			pstNode->mMatchedPatternsLen = mlen + 2;

		}
	
	};

	//write failure linkage
	std::queue<stTrieNode*> quene;	// 临时分配空间，无需使用PoolBufAlloc
	quene.push(pstTrieRoot);

	while(quene.size()>0)
	{
		stTrieNode* current,*parent,*down;
		int i;
		//fn_pop(&stack,&parent);
		parent = quene.front();
		quene.pop();

		for(i=0; i<CHARSIZE; i++)
		{
			current = parent->pstNext[i];
			if(current==NULL) continue;

			//	fn_push(&stack,current);
			quene.push(current);
			down = parent->pstFailure;

			while((down!=NULL)&&(down->pstNext[i]==NULL))
				down = down->pstFailure;

			if(down!=NULL)
			{
				current->pstFailure = down->pstNext[i];
				if (current->pstFailure->mMatchedPatterns != NULL)
				{
					//将current->pstFailure->mMatchendPatterns添加到current->mMatchedPatterns尾
					int oldlen = 0;
					int * pm;
					if (current->mMatchedPatterns == NULL)
					{
						oldlen = 0;
					}
					else
					{
						pm = current->mMatchedPatterns;
						while (*pm != -1)
						{
							oldlen ++;
							pm ++;
						}
					}
					int mlen = 0;
					pm = current->pstFailure->mMatchedPatterns;
					while (*pm != -1)
					{
						mlen ++;
						pm ++;
					}
					int newlen = mlen + oldlen;
					int * pnewMatched = new int[newlen + 1];	// 这里new的可能是临时的。全部完成后再一并转成PoolBufAlloc
					newed += sizeof(int) * (newlen + 1);
					if (oldlen > 0)
					{
						memcpy(pnewMatched, current->mMatchedPatterns, oldlen*sizeof(int));
					}
					memcpy(pnewMatched + oldlen, current->pstFailure->mMatchedPatterns, mlen*sizeof(int));
					pnewMatched[newlen] = -1;
					
					delete []current->mMatchedPatterns;
					deleted += current->mMatchedPatternsLen * sizeof(int);
					current->mMatchedPatterns = pnewMatched;
					current->mMatchedPatternsLen = newlen + 1;
				}
			}
			else
			{
				current->pstFailure = pstTrieRoot;
			}
		}

	}

	// point all the null link to root
	//quene.clear();
    while(!quene.empty()){
        quene.pop();
    }
	quene.push(pstTrieRoot);
	
	while(quene.size()>0)
	{
		stTrieNode* failure,* current;
		int i;
		
		current= quene.front();
		quene.pop();
		failure = current->pstFailure;

		// delete original mMatchedPatterns and replace it with poolbufed memory
		if(current->mMatchedPatternsLen != 0)
		{
			int *pnewMatched = (int *)alloc->allocate(current->mMatchedPatternsLen * sizeof(int));
			poolbufed += current->mMatchedPatternsLen * sizeof(int);
			memcpy(pnewMatched, current->mMatchedPatterns, current->mMatchedPatternsLen * sizeof(int));
			delete []current->mMatchedPatterns;
			current->mMatchedPatterns = pnewMatched;
		}

		for(i=0; i<CHARSIZE; i++)
		{
			if(current->pstNext[i]!=NULL)
				quene.push(current->pstNext[i]);
			else
			{
				if(failure==NULL)
					current->pstNext[i] = pstTrieRoot;
				else
					current->pstNext[i] = failure->pstNext[i];
			}
		}
	}

	ODS_UTILS_ALLOC_DEBUG("Aho Corasick init newed %lu, deleted %lu, poolbufed %lu. %s!\n",
						  newed, deleted, poolbufed, newed - deleted == poolbufed ? "OK" : "ERROR");

	//End changed!*/
	return 1;
}


int CPoolBuf_Aho_Corasick::search(const char * pBuff, int iSize, FDelegate pCallBackReportFound, void * pTag)
{
	if (pstTrieRoot == NULL)
	{
		return 0;
	}

	const char* pcCur,* pcEnd;
	stTrieNode* stState;
	pcCur = pBuff;
	stState = pstTrieRoot;

	pcEnd = pBuff + iSize;
	while(pcCur< pcEnd)
	{
		stState = stState->pstNext[(unsigned char)*pcCur];
		/*
		if(stState==NULL)
		{
			stState= pstTrieRoot;
			printf("error in automaton");
		}
		*/
		if ((stState->mMatchedPatterns != NULL) && (pCallBackReportFound != NULL))
		{
			int * pMatched = stState->mMatchedPatterns;
			while (*pMatched != -1)
			{
				int ret = pCallBackReportFound(*pMatched, (int)(pcCur - pBuff), pTag);
				if (ret < 0)
				{
					return ret;
				}

				pMatched ++;
			};
		};
		pcCur++;
	}
	return 1;
}
