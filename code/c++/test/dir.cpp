#include <string>
#include <dirent.h>  
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h> 
int main()
{
	std::string ts="111";
	char filename[64];
	snprintf(filename,64,"%s%d",ts.c_str(),30);
	printf("%s\n",filename);



	DIR *dirp;
    struct dirent *direntp;
    int stats;
	
    if((dirp=opendir("/search/wangwei/test"))==NULL)
    {
		printf("xxxx\n");
        exit(1);
    }
    while((direntp=readdir(dirp))!=NULL)
    {
		printf("zzz\n");
		//printf("d_name: %s\n",direntp->d_name);
/*
struct stat statbuf;          // 定义statbuf结构保存文件属性  
		lstat(direntp->d_name, &statbuf); // 获取下一级成员属性 
*/
		//if(!S_ISDIR(statbuf.st_mode)){  
		if(!(direntp->d_type & DT_DIR)){  
			printf("d_name: %s\n",direntp->d_name);
			struct stat statbuf;          // 定义statbuf结构保存文件属性  
			lstat(direntp->d_name, &statbuf); // 获取下一级成员属性 
			printf("time=%u\n",statbuf.st_mtime);

		}

    }
    return 0;

}
