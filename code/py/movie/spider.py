import urllib 
import time
import StringIO
from lxml import etree
import sys
import io
import os
import parse
import douban
import banyungong

target_file = sys.argv[1]
result_dir = sys.argv[2]

spider_list=[]
result_list=[]

def get_parser(target_url):
	
	idx =target_url.find("douban")
	if idx >0:
		return douban.DoubanParser() 
	idx = target_url.find("banyungong")
	if idx >0:
		return banyungong.BanyungongParser()

def get_tag(url):
	idx = url.rfind("/")
	if idx>0:
		idx2= url.find(".",idx+1)
		if idx2>0:
			return url[idx +1:idx2]
		return url[idx:]
	return None

def dump_result(file):
	index=0
	fp = io.open(file,"a")
	if len(result_list) > 10000:
		index=10000
	else:
		index=len(result_list)
	for i in range(1,index):
		link= result_list.pop()
		fp.write((link+"\n").decode("UTF-8"))

		fp.flush()
	fp.close()

for target_url in open(target_file,'r'):
	if len(target_url) <1:
		continue
	parser = get_parser(target_url)
	target_url= target_url.strip()
	dest = result_dir +'/' +get_tag(target_url) 
	if  os.path.exists(dest):
		continue
	os.mkdir(dest)
	result_file = dest+"/url"
	spider_list.append(target_url)
	print result_file
	#continue
	num=1
	while (len(spider_list)>0):
		target_url =spider_list.pop()
		res=urllib.urlopen(target_url).read()
		#parse_page(res)
		parser.dir_parse(res,spider_list,result_list)
		if len(result_list)> 10:
			dump_result('%s_%05d' % (result_file,num))
		num+=1
		time.sleep(1)
	if len(result_list)>0:		
		dump_result('%s_%05d' % (result_file,num))


