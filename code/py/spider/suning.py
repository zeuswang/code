import time
import base
import StringIO
from lxml import etree

import parse
import commodity
result_list_num = 10
class SuningParser(parse.PageParser):
	def dir_parse(self,page,result_list):
		#print page
		parser = etree.HTMLParser(recover=True)
		tree=etree.parse(StringIO.StringIO(page),parser)
		node=tree.xpath('//div[@id=\"proShow\"]')
		#print node
		table_list= node[0].xpath('./ul/li')
		for table in table_list:
			href= table.xpath('.//@href')
			if len(href)>0:
				#print href[0]
				req = base.CrawlRequest()
				req.url = href[0] 
				result_list.insert(0,req)
				if len(result_list) >= result_list_num:
				#if len(result_list) >1:
					break


	def detail_parse(self,page,item):
	#print page
		parser = etree.HTMLParser(recover=True)
		tree=etree.parse(StringIO.StringIO(page),parser)
		node_list=tree.xpath('//div[@class=\"product-main-title\"]')
		#print node_list
		for node in node_list:
			title=node.xpath('.//h1')
			for t in title:
				if t.text:
					item.title= t.text.strip()
				else:
					item.error=1
				break
			summary = node.xpath('.//h2')
			for s in summary:
				if s.text:
					item.summary= s.text.strip()
				else :
					item.error=1
				break
			#if item.title:
				#print item.title.encode("utf8")
			#if item.summary:
				#print item.summary.encode("utf8")
			break
		node_list=tree.xpath('//span[@id=\"netPrice\"]')
		#print "xxxxxx",node_list
		for node in node_list:
			em = node.xpath('.//em')
			for e in em:
				item.price = e.text
				break
			break
		#print item.price
		node_list=tree.xpath('//div[@id=\"PicView\"]')
		for node in node_list:
			img_src=node.xpath('.//@src')
			item.pic= img_src[0]
			#print img_src[0]
			break
	

	



