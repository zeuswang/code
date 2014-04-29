#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>

int main()
{
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
