import urllib 
import time
import StringIO
from lxml import etree
from scrapy.selector import HtmlXPathSelector
import sys
import os
import douban
import item
target_dir = sys.argv[1]
result_dir = sys.argv[2]
if result_dir =="": 
	sys.exit()
spider_list=[]
result_list=[]
parser =douban.DoubanParser()
def douban_dir_func(page):
	#print page
	parser = etree.HTMLParser(recover=True)
	tree=etree.parse(StringIO.StringIO(page),parser)
	node=tree.xpath('//*[contains(@class,\'article\')]')
	table_list= node[0].xpath('./div[2]//table')
	for table in table_list:
		href= table.xpath('./tr[1]/td[2]//@href')
		if len(href)>0:
			#print href[0]
			result_list.insert(0,href[0])
	next_link =tree.xpath('//*[contains(@class,\'next\')]')
	print next_link
	if len(next_link)==0:
		return
	link=next_link[0].xpath('.//@href')
	if len(link) >0:
		print link[0] 
		spider_list.append(link[0])

	print len(next_link)


def parse_page(res,it):
	parser.detail_parse(res,it)
def dump_result():
	index=0
	if len(result_list) > 10000:
		index=10000
	else:
		index=len(result_list)
	for i in range(1,index):
		link= result_list.pop()
		print link

folders = os.listdir(target_dir)
for name in folders:
	print name
 	res_fp = open(result_dir +"/"+name,"a")
	for line in open(target_dir +name,'r'):
		url=line.strip();
		spider_list.append(url)
		while (len(spider_list)>0):
			target_url =spider_list.pop()
			res=urllib.urlopen(target_url).read()
			it = item.Item()
			parse_page(res,it)
			res_fp.write('%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' % (it.cname,it.ename,it.actors,it.director,it.writer,it.location,it.type,it.date,it.runtime,it.rate,it.votes))
			res_fp.flush()
			time.sleep(3)
		
		dump_result()
	res_fp.close()
