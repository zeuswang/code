# coding=utf8
import time
import StringIO
from pyquery import PyQuery as pyq  
import parse
import item
result_list_num = 10
"""汉字处理的工具:
判断unicode是否是汉字，数字，英文，或者其他字符。
全角符号转半角符号。"""
 
def is_chinese(uchar):
	"""判断一个unicode是否是汉字"""
	if uchar >= u'\u4e00' and uchar<=u'\u9fa5':
		return True
	else:
		return False
 
def is_number(uchar):
	"""判断一个unicode是否是数字"""
	if uchar >= u'\u0030' and uchar<=u'\u0039':
		return True
	else:
		return False
 
def is_alphabet(uchar):
	"""判断一个unicode是否是英文字母"""
	if (uchar >= u'\u0041' and uchar<=u'\u005a') or (uchar >= u'\u0061' and uchar<=u'\u007a'):
		return True
	else:
		return False
 
def is_other(uchar):
	"""判断是否非汉字，数字和英文字符"""
	if not (is_chinese(uchar) or is_number(uchar) or is_alphabet(uchar)):
		return True
	else:
		return False

class BanyungongParser(parse.PageParser):
	def dir_parse(self,page,spider_list,result_list):
		print page
		doc = pyq(page)
		tmp = doc('table[class=tableList]')
		trl = tmp('tr')
		for v in trl:
			td= pyq(v)('td[class=title]')
			a = td('a')
			name =  a.text().encode("UTF-8").decode("UTF-8")
			ename =""
			print name
			if len(name)>1:	
				for uchar in name:
					#print uchar
					if  is_alphabet(uchar) :
						ename += uchar
					#elif uchar =='.' or uchar ==' ' or uchar =='&':
						#ename += uchar
					elif (uchar =='(' or is_number(uchar) ) and len(ename)>2:
						break
				print "xxxx",ename

				link =  "http://banyungong.net/" + a.attr('href')
				result_list.append((ename.lower() +"," +link).encode("UTF-8"))
		

		return ""

	def detail_parse(self,page,it):
		print "######"
		return ""





