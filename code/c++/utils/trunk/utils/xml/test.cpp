#include "SXml.h"
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

	SXmlDocument doc;
	doc.Parse(xml,"utf-8");
	SXmlNode old_node=doc.GetRoot().GetChildren();
		for(SXmlNode i=old_node;!i.Null();i=i.GetNext()){
			printf("term:%s",i.GetChildren("term").GetContent().c_str());	
		}

}
