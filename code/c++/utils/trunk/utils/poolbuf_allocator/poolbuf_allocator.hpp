#pragma once

//#define ODS_UTILS_ALLOC_DEBUG printf
#define ODS_UTILS_ALLOC_DEBUG(...)

#include <memory>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <assert.h>
#include <ext/hash_map>

typedef unsigned char BYTE;

namespace ods_utils
{

class PoolBufAlloc
{
public:
	typedef size_t     size_type;

	PoolBufAlloc():
		m_alloced(0),
		m_freed(0),
		m_occupy(0)
	{
	}

private:
	// disable copy
	PoolBufAlloc(const PoolBufAlloc&);
	PoolBufAlloc &operator =(const PoolBufAlloc&);

public:
	virtual ~PoolBufAlloc() throw()
	{
	}

	// NB: __n is permitted to be 0.  The C++ standard says nothing
	// about what the return value is when __n == 0.
	virtual void *allocate(size_type __n = 1, const void* = 0) = 0;

	// __p is not permitted to be a null pointer.
	virtual void deallocate(void *__p, size_type) = 0;

	virtual size_type
	max_size() const throw()
	{ return size_type(-1);}

	// A few helpers 
	const char *allocate_string(const std::string &str)
	{
		char *buf = (char *)allocate(str.length() + 1);
		strcpy(buf, str.c_str());
		return buf;
	}

	const char *allocate_string(const char *str)
	{
		char *buf = (char *)allocate(strlen(str) + 1);
		strcpy(buf, str);
		return buf;
	}

	// statistic
	virtual size_type getAllocated()
	{
		return m_alloced;
	}

	virtual size_type getFreed()
	{
		return m_freed;
	}

	virtual size_type getOccupy()
	{
		return m_occupy;
	}

protected:
	size_type m_alloced;
	size_type m_freed;
	size_type m_occupy;
};

inline bool
operator==(const PoolBufAlloc &o1, const PoolBufAlloc &o2)
{ return &o1 == &o2;}

inline bool
operator!=(const PoolBufAlloc &o1, const PoolBufAlloc &o2)
{ return &o1 != &o2;}

#define ODS_UTILS_MEM_PAD 0xcdcdcdcdu
#define ODS_UTILS_MEM_DEL_PAD 0xddu

class SinglePoolBufAlloc : public PoolBufAlloc
{
public:
	typedef size_t     size_type;

	SinglePoolBufAlloc(size_type blockSize = 128 * 1024 * 1024):
		m_head(NULL),
		m_cur(NULL),
		m_largeHead(NULL),
		m_largeCur(NULL),
		m_blockSize(blockSize),
		m_allocp(NULL),
		m_tail(NULL)
	{
	}

//private:
//    // disable copy
//    SinglePoolBufAlloc(const SinglePoolBufAlloc&);
//    SinglePoolBufAlloc &operator =(const SinglePoolBufAlloc&);
//
public:
	virtual ~SinglePoolBufAlloc() throw()
	{
		Refresh(false);
	}

#define AllocItemExSize ((size_t)&(((AllocItem*)NULL)->data[0]) + sizeof(((AllocItem*)NULL)->pad1))
#define AllocDataOffset ((size_t)&(((AllocItem*)NULL)->data[0]))
	// NB: __n is permitted to be 0.  The C++ standard says nothing
	// about what the return value is when __n == 0.
	virtual void *allocate(size_type __n = 1, const void* = 0)
	{
		if (__builtin_expect(__n > this->max_size(), false))
			std::__throw_bad_alloc();

		if(0 == __n)
			return (void *)&m_zero.data[0];

		if(NULL == m_head)	// no space allocated
		{
			m_occupy += m_blockSize;
			m_head = (BlockItem *)malloc(m_blockSize);
			m_head->size = m_blockSize;
			m_head->next = NULL;
			m_cur = m_tail = m_head;
			m_allocp = &m_head->data[0];
		}

		size_type actSize = (__n + AllocItemExSize + 3) / 4 * 4;	// make pad2 pack by 4

		AllocItem *ret = NULL;
		if(&m_head->data[0] - (BYTE *)m_head + actSize > m_blockSize)   // too big even for a whole block
		{
			size_type largeBlockSize = &m_head->data[0] - (BYTE *)m_head + actSize; // actual block size needed
			fprintf(stderr, "SinglePoolBufAlloc WARNING :allocated LARGE block size of %lu bytes\n",
					largeBlockSize);
			if(NULL == m_largeHead)
			{
				m_largeHead = m_largeCur = (BlockItem *)malloc(largeBlockSize);
			}
			else
			{
				m_largeCur->next = (BlockItem *)malloc(largeBlockSize);
				m_largeCur = m_largeCur->next;
			}
			// now m_largeCur must be the newly allocated large block
			if(!m_largeCur)
				return NULL;
			m_occupy += largeBlockSize;
			m_largeCur->size = largeBlockSize;
			m_largeCur->next = NULL;
			ret = (AllocItem *)&m_largeCur->data[0];
		}
		else	// not a large alloc
		{
			if(m_allocp + actSize - (BYTE *)m_cur > m_cur->size)	// current block not sufficient
			{
				if(!m_cur->next)
				{
					// allocate a new block
					m_occupy += m_blockSize;
					m_cur->next = (BlockItem *)malloc(m_blockSize);
					fprintf(stderr, "SinglePoolBufAlloc allocated block size of %lu bytes %p\n", m_blockSize, m_cur->next);
					m_cur->next->size = m_blockSize;
					m_cur->next->next = NULL;
					m_tail =m_cur->next;
				}
				m_cur = m_cur->next;
				m_allocp = &m_cur->data[0];
			}
//  	assert(m_allocp + actSize - (BYTE *)m_cur <= m_cur->size);
			ret = (AllocItem *)m_allocp;
			m_allocp += actSize;
		}
		ret->size = actSize - AllocItemExSize;
		ret->pad1 = *(unsigned int *)(&ret->data[0] + ret->size) = ODS_UTILS_MEM_PAD;	// add paddings

		m_alloced += __n;

		ODS_UTILS_ALLOC_DEBUG("PoolBufAlloc::allocate() with __n(%u), actSize(%u), effSize(%u)\n",
							  __n, actSize, ret->size);

		return static_cast<void*>(&ret->data[0]);
	}

	// __p is not permitted to be a null pointer.
	virtual void deallocate(void *__p, size_type)
	{
		AllocItem *item = (AllocItem *)((BYTE *)__p - AllocDataOffset);
		m_freed += item->size + AllocItemExSize;

		if(item != &m_zero && (item->pad1 != ODS_UTILS_MEM_PAD ||
		   *(unsigned int *)(&item->data[0] + item->size) != ODS_UTILS_MEM_PAD))
		{
			fprintf(stderr, "ERROR: PoolBufAlloc::deallocate() detected buffer overflow at %p\n", __p);
		}

		ODS_UTILS_ALLOC_DEBUG("PoolBufAlloc::deallocate() with actSize(%u), effSize(%u)\n",
							  item->size + AllocItemExSize, item->size);
//  	::operator delete((BYTE *)item);
	}

	// drop all allocated memory. return them to OS if reserve==false
	void Refresh(bool reserve = false)
	{
		m_alloced = m_freed = 0;
		if(reserve)
		{
			m_cur = m_head;
			m_allocp = &m_head->data[0];
		}
		else
		{
			m_cur = m_head;
			while(m_cur)
			{
				m_head = m_cur->next;
				fprintf(stderr, "SinglePoolBufAlloc freed block size of %lu bytes %p\n", m_cur->size, m_cur);
				memset(m_cur, ODS_UTILS_MEM_DEL_PAD, m_cur->size);	// write protect bytes
				free(m_cur);
				m_cur = m_head;
			}
			m_cur = m_head = m_tail = NULL;
			m_allocp = NULL;
			m_occupy = 0;
		}
		// free large blocks whether reserve or not
		m_largeCur = m_largeHead;
		while(m_largeCur)
		{
			m_largeHead = m_largeCur->next;
			fprintf(stderr, "SinglePoolBufAlloc freed LARGE block size of %lu bytes %p\n", m_largeCur->size, m_largeCur);
			memset(m_largeCur, ODS_UTILS_MEM_DEL_PAD, m_largeCur->size);	// write protect bytes
			free(m_largeCur);
			m_largeCur = m_largeHead;
		}
		m_largeCur = m_largeHead = NULL;
	}

protected:
	struct BlockItem	// item for buffer block
	{
		size_t size;
		BlockItem *next;
		BYTE data[1];
	};
	struct AllocItem
	{
		unsigned int pad1;
		size_type size;
		BYTE data[1];
	};
	BlockItem *m_head;
	BlockItem *m_tail;
	BlockItem *m_cur;
	BlockItem *m_largeHead;
	BlockItem *m_largeCur;
	BYTE *m_allocp;
	static AllocItem m_zero;	// zero sized item
	size_type m_blockSize;
};

/**
 * class PoolBufAllocAdapter 
 * STL allocator 兼容的 PoolBufAlloc 适配器。使用这 
 * 个类让STL容器等可以应用PoolBufAlloc分配内存。构造 
 * PoolBufAllocAdapter 对象时必须提供一个 PoolBufAlloc 
 * 对象指针用于实际的内存分配。 
 * 
 * @author Mingliang (2011-12-06)
 */
template<typename _Tp>
	class PoolBufAllocAdapter
{
public:
	typedef size_t     size_type;
	typedef ptrdiff_t  difference_type;
	typedef _Tp*       pointer;
	typedef const _Tp* const_pointer;
	typedef _Tp&       reference;
	typedef const _Tp& const_reference;
	typedef _Tp        value_type;

	template<typename _Tp1>
	struct rebind
	{
		typedef PoolBufAllocAdapter<_Tp1> other;
	};

private:
	PoolBufAllocAdapter() throw();	// disable default construct

public:
	PoolBufAllocAdapter(PoolBufAlloc *alloc) throw():m_interAlloc(alloc) {}

	template<typename _Tp1>
	PoolBufAllocAdapter(const PoolBufAllocAdapter<_Tp1>& alloc1) throw()
		:m_interAlloc(alloc1.GetAlloc())
	{
	}

	~PoolBufAllocAdapter() throw() {}

	pointer
	address(reference __x) const { return &__x;}

	const_pointer
	address(const_reference __x) const { return &__x;}

	// NB: __n is permitted to be 0.  The C++ standard says nothing
	// about what the return value is when __n == 0.
	pointer
	allocate(size_type __n, const void* = 0)
	{ 
		if (__builtin_expect(__n > this->max_size(), false))
			std::__throw_bad_alloc();

		return static_cast<_Tp*>(m_interAlloc->allocate(__n * sizeof(_Tp)));
	}

	// __p is not permitted to be a null pointer.
	void
	deallocate(pointer __p, size_type __n)
	{ m_interAlloc->deallocate(__p, __n);}

	size_type
	max_size() const throw() 
	{ return m_interAlloc->max_size() / sizeof(_Tp);}

	// _GLIBCXX_RESOLVE_LIB_DEFECTS
	// 402. wrong new expression in [some_] allocator::construct
	void 
	construct(pointer __p, const _Tp& __val) 
	{ ::new(__p) _Tp(__val);}

	void 
	destroy(pointer __p) { __p->~_Tp();}

	pointer NewObj()	// malloc and construct (call default constructor)
	{
		pointer obj = allocate(1);
		::new(obj) _Tp();
		return obj;
	}

	pointer NewObjAlloc()	// malloc and construct (call constructor with m_interAlloc as parameter)
	{
		pointer obj = allocate(1);
		::new(obj) _Tp(m_interAlloc);
		return obj;
	}

	bool operator==(const PoolBufAllocAdapter<_Tp> &r)
	{
		return m_interAlloc == r.m_interAlloc;
	}

	bool operator!=(const PoolBufAllocAdapter<_Tp> &r)
	{
		return m_interAlloc != r.m_interAlloc;
	}

	PoolBufAlloc *GetAlloc() const
	{
		return m_interAlloc;
	}

protected:
	PoolBufAlloc *m_interAlloc;
};

// do these so you can use PoolBufXXX<>::type xxx; to simply declare STL objects using PoolBufAlloc
// PoolBufMap<key, val>::type map1
template<typename _Key, typename _Tp>
	struct PoolBufMap
{
	typedef std::map<_Key, _Tp, std::less<_Key>,
	PoolBufAllocAdapter<std::pair<const _Key, _Tp> > > type;
};
// map for _Key=const char*
struct str_less
{
	size_t operator()( const char *str1, const char *str2) const
	{
		return strcmp(str1, str2) < 0;
	}
};
struct str_hash
{
	size_t operator()( const char *str) const
	{
		return __gnu_cxx::hash<const char*>()(str);
	}
};
struct str_equal
{
	size_t operator()( const char *str1, const char *str2) const
	{
		return strcmp(str1, str2) == 0;
	}
};
template<typename _Tp>
	struct PoolBufMapCharP
{
	typedef std::map<const char *, _Tp, str_less,
	PoolBufAllocAdapter<std::pair<const char *, _Tp> > > type;
};
//typedef __gnu_cxx::hash_map<const char *, WordParamP, str_hash,
//str_equal, ods_utils::PoolBufAllocAdapter<WordParamP> > KeyHashMap;
template<typename _Tp>
	struct PoolBufHashmapCharP
{
	typedef __gnu_cxx::hash_map<const char *, _Tp, str_hash, str_equal,
	PoolBufAllocAdapter<_Tp> > type;
};

// string
typedef std::basic_string<char, std::char_traits<char>, PoolBufAllocAdapter<char> > PoolBufString;
// vector
template<typename _Tp>
	struct PoolBufVector
{
	typedef std::vector<_Tp, PoolBufAllocAdapter<_Tp> > type;
};
// set
template<typename _Tp>
	struct PoolBufSet
{
	typedef std::set<_Tp, std::less<_Tp>, PoolBufAllocAdapter<_Tp> > type;
};

//// special form for vector<vector<> >. _Tp only allows vector<..>
//template<typename _Tp>
//    class PoolBufVectorVector : public std::vector<_Tp, PoolBufAllocAdapter<_Tp> >
//{
//};

//template<typename _Tp>
//inline bool
//operator==(const poolbuf_allocator<_Tp> &l, const poolbuf_allocator<_Tp> &r)
//{ return l.m_interAlloc ;}
//
//template<typename _Tp>
//inline bool
//operator!=(const poolbuf_allocator<_Tp>&, const poolbuf_allocator<_Tp>&)
//{ return false;}

class DoublePoolBufAlloc : public PoolBufAlloc
{
public:
	DoublePoolBufAlloc(size_type blockSize = 128 * 1024 * 1024):
		alloc1(blockSize),
		alloc2(blockSize)
	{
		m_use = &alloc1;
		m_old = &alloc2;
	}

	// NB: __n is permitted to be 0.  The C++ standard says nothing
	// about what the return value is when __n == 0.
	virtual void *allocate(size_type __n = 1, const void* __p = 0)
	{
		return m_use->allocate(__n, __p);
	}

	// __p is not permitted to be a null pointer.
	virtual void deallocate(void *__p, size_type __n)
	{
		return m_use->deallocate(__p, __n);
	}

	// drop all allocated memory of old buffer. return them to OS if reserve==false
	void Refresh(bool reserve = false)
	{
		m_old->Refresh(reserve);
	}

	// switch buffer, call this when should allocate memory on the other buffer
	void Switch()
	{
		// swap m_use & m_new
		fprintf(stderr, "DoublePoolBufAlloc switched pool\n");
		SinglePoolBufAlloc *tmp = m_use;
		m_use = m_old;
		m_old = tmp;
	}

	// giveup (refresh) newly allocated memory and switch back
	void Giveup(bool reserve = false)
	{
		fprintf(stderr, "DoublePoolBufAlloc gave up pool\n");
		Switch();	// must switch back first
		Refresh(reserve);
	}

	virtual size_type getAllocated()
	{
		return m_use->getAllocated() + m_old->getAllocated();
	}

	virtual size_type getFreed()
	{
		return m_use->getFreed() + m_old->getFreed();
	}

	virtual size_type getOccupy()
	{
		return m_use->getOccupy() + m_old->getOccupy();
	}

protected:
	SinglePoolBufAlloc alloc1, alloc2;
	SinglePoolBufAlloc *m_use, *m_old;
};

}
