# coding=utf8
import time
import base
import StringIO
from pyquery import PyQuery as pyq  
import parse
import item
result_list_num = 10
class BanyungongParser(parse.PageParser):
	def dir_parse(self,page,spider_list,result_list):
		print page
		doc = pyq(page)
		tmp = doc('table[class=tableList]')
		trl = tmp('tr')
		for v in trl:
			td= pyq(v)('td[class=title]')
			a = td('a')
			name =  a.text()
			if len(name)>1:	
				link =  "http://banyungong.net/" + a.attr('href')
				result_list.append((name +"," +link).encode("UTF-8"))
		

		return ""

	def detail_parse(self,page,it):
		print "######"
		return ""





