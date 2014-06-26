import urllib 
import time
import StringIO
from lxml import etree
from scrapy.selector import HtmlXPathSelector
target_file="target/detail/douban.detail"
spider_list=[]
result_list=[]

class MovieInfo:
	name=""
	director=[]
	actor=[]
	douban_rank=0
	type=""
	time=[]

def douban_detail_func(page):
	#print page
	parser = etree.HTMLParser(recover=True)
	tree=etree.parse(StringIO.StringIO(page),parser)
	#node=tree.xpath('//*[contains(@id,\'content\')]')
	node=tree.xpath('//div[@id="content"]')
	if len(node) ==0:
		return 
	movieinfo= MovieInfo()

	title= node[0].xpath('h1//span[@property="v:itemreviewed"]')
	if len(title)>0:
		 movieinfo.name = title[0].text

	info_node = node[0].xpath('.//div[@id="info"]')
	for info in infonode:
		span_node= info.xpath('.//span')
		link_list=span_node[0].xpath('..//a')
		for link in link_list:
			movieinfo.director.append(link.text);
		link_list=span_node[2].xpath('..//a')
		for link in link_list:
			movieinfo.actor.append(link.text);

		time_node = info.xpath('.//span[@property="v:initialReleaseDate"]')
		for time in time_node:
			movieinfo.time.append(time.text)


	rank_node = node[0].xpath('.//div[@id="interest_sectl"]')
	rank = node[0].xpath('.//strong[@class="ll rating_num"]')
	if len(rank)>0:
		movieinfo.douban_rank=rank[0].text

	
def parse_page(res):
	douban_detail_func(res)

def dump_result():
	index=0
	if len(result_list) > 10000:
		index=10000
	else:
		index=len(result_list)
	for i in range(1,index):
		link= result_list.pop()
		print link

for line in open(target_file,'r'):
	url=line.strip();
	res=urllib.urlopen(url).read()
	parse_page(res)
	if len(result_list)> 10000:
		dump_result()

	time.sleep(1)
		
dump_result()


'''
url='http://banyungong.net/category/101.html'
#url='http://www.baidu.com'
res=urllib.urlopen(url).read()
res=res.replace('\r','')
#print res
res2='<html><body><p>hello</p></body></html>'
parser = etree.HTMLParser(recover=True)
tree=etree.parse(StringIO.StringIO(res),parser)
node=tree.xpath('/html/body/form/*[contains(@class,\'DivOuter\')]')
print node
r=node[0].xpath('div[2]/table')
tr=r[0].xpath('tr')
for tube in tr:
	time = tube.xpath('./td[1]//text()')
	if len(time)>0:
		print time[0]
	title = tube.xpath('./td[3]//text()')
	if len(title)>0:
		print title[0].encode("utf8")
	url = tube.xpath('./td[3]//@href')
	if len(url)>0:
		print url[0]

#print len(tr)
#print r[0].text
'''
