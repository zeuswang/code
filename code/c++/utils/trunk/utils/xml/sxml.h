#ifndef SXML_H
#define SXML_H
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <string>
#include <string.h>

int utf16_to_utf8(const std::string &src, std::string &result);
int utf8_to_utf16(const std::string &src, std::string &result);
int utf16_to_gbk(const std::string &src, std::string &result);
int gbk_to_utf16(const std::string &src, std::string &result);
int utf8_to_gbk(const std::string &src, std::string &result);
int gbk_to_utf8(const std::string &src, std::string &result);

#define USE_LIBXML
enum {
   /* XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_TEXT_NODE = 3,
    XML_CDATA_SECTION_NODE = 4,
    XML_ENTITY_REF_NODE = 5,
    XML_ENTITY_NODE = 6,
    XML_PI_NODE = 7,
    XML_COMMENT_NODE = 8,
    XML_DOCUMENT_NODE = 9,
    XML_DOCUMENT_TYPE_NODE = 10,
    XML_DOCUMENT_FRAG_NODE = 11,
    XML_NOTATION_NODE = 12,
    XML_HTML_DOCUMENT_NODE = 13,
    XML_DTD_NODE = 14,
    XML_ELEMENT_DECL = 15,
    XML_ATTRIBUTE_DECL = 16,
    XML_ENTITY_DECL = 17,
    XML_NAMESPACE_DECL = 18,
    XML_XINCLUDE_START = 19,
    XML_XINCLUDE_END = 20,
    XML_DOCB_DOCUMENT_NODE = 21,
    */
    XML_UNKNOWN=99,
};

class SXmlNode
{
friend class SXmlDocument;
public:
  SXmlNode(){
    _node=NULL;
  }
  
	std::string GetName();
	void SetName(const std::string& name);
	SXmlNode GetChildren();
	SXmlNode GetChildren(std::string);
	std::string GetAttribute(std::string name);
	int SetAttribute(std::string name,std::string value);
	SXmlNode GetCopy();
	SXmlNode GetNext();
	int SetContent(std::string);
	std::string GetContent();
	std::string GetNodeListString(class SXmlDocument&);
	int NewAttribute(std::string name,std::string value);
	SXmlNode AddChild(const SXmlNode& node);	
	SXmlNode NewChild(const std::string& name,const std::string& content);
	void SetNodeName(const std::string& tagName);
	int SetContentInCData(std::string value);
	int GetType();
	bool HasAttribute(const std::string& name);
	bool Null();
	void Remove();
private:
#ifdef USE_LIBXML
	xmlNodePtr _node;
#endif
	
};
class SXmlDocument
{
friend class SXmlNode;
public:
	SXmlDocument(){_doc=NULL;};
	virtual ~SXmlDocument();
	int Parse(const std::string& str,std::string  encoding);
	SXmlNode GetRoot();
	int SaveToString(std::string &str);
  int SaveToStringWithFormat(std::string &str,int format);
	int SaveToStringWithEncode(std::string &str,std::string out_encode);
  int SaveToStringWithEncodeFormat(std::string &str,std::string out_encode,int format);
	//void Free();
private:
	int ChangeEncodeDeclaration(std::string in,std::string &out,std::string src,std::string des);
	std::string _encoding;
#ifdef USE_LIBXML
	xmlDocPtr _doc;
#endif
};

#endif

