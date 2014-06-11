#include "sxml.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
int main(){
	std::string xml=
"<?xml version=\"1.0\" encoding=\"gbk\"?> \
<DOCUMENT> \
<item> \
    <key ><![CDATA[beijing]]></key> \
    <display> \
       <title><![CDATA[XXXXXXXXXXXXXXXX]]></title> \
        <url><![CDATA[http://www.weather.com.cn/weather/101010100.shtml]]></url> \
   </display> \
<classid>207302</classid> \
<term><![CDATA[(beijing)]]></term>   \ 
</item>  \
<item> \
    <key ><![CDATA[beijing2]]></key> \
    <display> \
       <title><![CDATA[XXXXXXXXXXXXXXXX]]></title> \
        <url><![CDATA[http://www.weather.com.cn/weather/101010100.shtml]]></url> \
   </display> \
<classid>207302</classid> \
<term><![CDATA[(beijing2)]]></term>   \ 
</item>  \
</DOCUMENT>";
/*
	SXmlDocument doc;
	doc.Parse(xml,"utf-8");
	SXmlNode old_node=doc.GetRoot().GetChildren();
		for(SXmlNode i=old_node;!i.Null();i=i.GetNext()){
			printf("term:%s",i.GetChildren("term").GetContent().c_str());	
		}
*/
	char buf[4096];
	int fd = open("1.test",O_RDONLY);
	std::string tmp;
	while (1)
	{
		int ret =read(fd, buf,4096);
		if (ret <=0)
			break;
		tmp += std::string(buf,ret);
	}
	//printf("str=%s\n",tmp.c_str());
	SXmlDocument doc;
	doc.Parse(tmp,"gbk");
	SXmlNode old_node=doc.GetRoot().GetChildren();
		for(SXmlNode i=old_node;!i.Null();i=i.GetNext()){
			printf("name=%s\n",i.GetName().c_str());	
			if (i.GetName() == "doc")
			{
				printf("title=%s\n",i.GetChildren("item").GetChildren("display").GetChildren("title").GetContent().c_str()); 	
				printf("TRank=%s\n",i.GetChildren("rank").GetAttribute("TRank").c_str()); 	

			}
			//printf("title:%s\n",i.GetChildren("title").GetContent().c_str());	
		}


}
