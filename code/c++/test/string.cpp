#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
char HEAD[]="\r\n\r\n";

int find(char * buf,int l) {
	int len = l -3;
	if (0 > len) {
		len = 0;
	}
	char * head = buf;
	//rdlen = rdlen -3;
	for (int i=0; i< len; ++i, ++head) {
		if ( (*(int *)HEAD) == (*(int *)head)) {
			return i+4;
		}
	}
	return -1;
}
int main()
{
char  str[] ="abcc\r\n\r\n123";
int ret = find(str,11);
printf("ret=%d\n",ret);
double tt=9.123456;
tt=tt*tt;
printf("%f\n",tt);
uint64_t pp = tt*(10^9);
printf("%d\n",sizeof(double));
	std::string ts="111";
	char filename[64];
	snprintf(filename,64,"%s%d",ts.c_str(),30);
	printf("%s\n",filename);
	//char *p = "46744073709551616";
	char *p = "10";

std::stringstream strValue;

strValue << p;
printf("%s\n",strValue.str().c_str());

//uint64_t  value;
uint8_t  value;

strValue >> value;
std::cout << value << std::endl;
std::cout << atoi(p) << std::endl;
}
