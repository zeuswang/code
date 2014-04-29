// Match.cpp: implementation of the CMatch class.
//
//////////////////////////////////////////////////////////////////////
#include "PoolBufMatch.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>

CPoolBufMatch::CPoolBufMatch()
{
	m_ppPatterns = NULL;
	m_pPatLen = NULL;
	m_num=0;
}

CPoolBufMatch::~CPoolBufMatch()
{
}

////#include "Tool.h"
//int CMatch::copyPatterns(int iNumber, const int pLen[], const char * pPatterns[])
//{
//	m_num = iNumber;
//	m_ppPatterns = new char *[m_num];
//	m_pPatLen = new int[m_num];
//	for(int i = 0;i < m_num; i++)
//	{
//		char * pkey = new char[pLen[i]];
//		memcpy(pkey, pPatterns[i], pLen[i]);
//
//		m_ppPatterns[i] = pkey;
//		m_pPatLen[i] = pLen[i];
//	}
//
//	return m_num;
//}
//
//void CMatch::releaseCopy()
//{
//	for (int i = 0; i < m_num; i ++)
//	{
//		delete []m_ppPatterns[i];
//	}
//	delete []m_ppPatterns;
//	m_ppPatterns = NULL;
//	delete []m_pPatLen;
//	m_pPatLen = NULL;
//	m_num = 0;
//}

