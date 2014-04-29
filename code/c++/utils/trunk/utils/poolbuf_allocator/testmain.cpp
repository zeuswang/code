#include "poolbuf_allocator.hpp"
#include <vector>
#include <map>
#include <string>

using namespace ods_utils;
using namespace std;

struct teststruct
{
	int test;
	bool operator <(teststruct &right){return test < right.test;}
};

int main(int, char **, char **)
{
	SinglePoolBufAlloc alloc;
	PoolBufAllocAdapter<char> alloc2(&alloc);
	PoolBufMap<int, teststruct>::type *test = new PoolBufMap<int, teststruct>::type(
		PoolBufMap<int, teststruct>::type::key_compare(),
		PoolBufMap<int, teststruct>::type::allocator_type(&alloc));
	teststruct a = {0};
	(*test)[0] = a;
	(*test)[1] = a;
	(*test)[2] = a;
	return 0;
}
