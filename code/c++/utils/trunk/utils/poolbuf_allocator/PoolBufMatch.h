//��ؼ���ƥ���㷨����CMatch
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

//����������ÿ���ҵ�һ���ؼ���ģʽ��ʱ���Զ����ûص�����
//iPatternIndex: ƥ��Ĺؼ���ID(��0���ؼ��ʸ���-1)
//iFoundPos: �ؼ���ƥ���ƫ����(���ֽ�Ϊ��λ)��ƫ����������ƥ��ؼ��ʵ�ͷ����Ҳ������β�����Ӿ����ʵ���㷨
//pTag: �û���search()���ݹ����Ĳ���
//�������������ֵ<0������ֹ�����ı�ɨ�������˳�
typedef int (* FDelegate)(int iPatternIndex, int iFoundPos, void * pTag);


class CPoolBufMatch  
{
	
public:
	CPoolBufMatch();
	virtual ~CPoolBufMatch();

	//��ʼ������
	//iNumber: �ؼ��ʸ���
	//pLen[]: �ؼ��ʳ�������
	//pPatterns[]: �ؼ���ָ������
	//����ֵС��0����ʼ������
	//��ʼ���������غ��û������ͷŹؼ���������ռ�Ĵ洢
	virtual int initialize(int iNumber, const int pLen[], const char * pPatterns[], ods_utils::PoolBufAlloc *alloc) = 0;

	//���溯����Ҫ����ʵ��
	//���ڴ���ɨ��pBuff,iSizeɨ��ؼ���
	virtual int search(const char * pBuff, int iSize, FDelegate pCallBackReportFound, void * pTag) = 0;

	virtual void clear() = 0;

protected:

	//�¿��ٻ�����������һ�ݹؼ��������ı�
//	int copyPatterns(int iNumber, const int pLen[], const char * pPatterns[]);
//	void releaseCopy();

protected:
	//����ؼ����ı�������
	char ** m_ppPatterns;
	//����ÿ���ؼ��ʳ��ȵ�����
	int   * m_pPatLen;

	//���ڹؼ����п��ܰ��������Ʊ���
	//���Բ���ʹ��strlenȡ�ؼ��ʳ���
	int   m_num;//�ؼ��ʸ���

};

#endif //POOLBUF_MATCH_H_
