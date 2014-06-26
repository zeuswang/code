# coding=utf8
import time
import base
import StringIO
from pyquery import PyQuery as pyq  
from lxml import etree
import parse
import item
result_list_num = 10
class DoubanParser(parse.PageParser):
	def dir_parse(self,page,spider_list,result_list):
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

		return ""

	def detail_parse(self,page,it):
	#print page
		doc = pyq(page)
		tmp = doc('div[id=info]')
		for v in tmp:
			#print pyq(v).html().encode("UTF-8")
			info= pyq(v).text().encode("UTF-8")
			#print info
			idx = str(info).find("制片国家/地区:")
			idx2 = str(info).find("语言:")
			if idx >0 and idx2 >0:
				it.location = info[idx +len("制片国家/地区:"):idx2]
#				print location
			idx = str(info).find("类型:")
			idx2 = str(info).find("官方网站")
			if idx >0 and idx2 >0:
				it.type = info[idx +len("类型:"):idx2]

			idx = str(info).find("编剧")
			idx2 = str(info).find("主演")
			if idx >0 and idx2 >0:
				it.writer = info[idx +len("编剧"):idx2]

			runtv = pyq(v)('span[property=\'v:runtime\']')
			if runtv is not None:
				it.runtime = runtv.text().encode("UTF-8")
			director = pyq(v)('a[rel=\'v:directedBy\']')
			if director is not None:
				it.director = director.text().encode("UTF-8")

			ac = pyq(v)('a[rel=\'v:starring\']')
			for actor in ac:
				it.actors += "/"+pyq(actor).text().encode("UTF-8")

			st = pyq(v)('span[property=\'v:initialReleaseDate\']')
			if st is not None:
				it.date = st.text().encode("UTF-8")

		rate = pyq(page)('strong[property=\'v:average\']')
		if rate is not None:
			it.rate = rate.text().encode("UTF-8")
		votes = pyq(page)('span[property=\'v:votes\']')
		if votes is not None:
			it.votes = votes.text().encode("UTF-8")

		namestr = pyq(page)('meta[name=\'keywords\']')
		if len(namestr)>0:
			s = namestr.attr('content').encode("UTF-8").split(",")	
			it.cname= s[0]
			it.ename= s[1]

		print "######"





