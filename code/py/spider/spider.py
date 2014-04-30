#-*- coding: UTF-8 -*-
import log
import base
import urllib2
import codecs
import adid
import urllib
import os
import sys
import time
import StringIO
import Image
import cStringIO
from lxml import etree
import suning
import commodity
from ghost import Ghost

target_file=sys.argv[1]
root_dir=sys.argv[2]
top_num = 4


def p(f):
	    print '%s.%s(): %s' % (f.__module__, f.__name__, f())
p(sys.getdefaultencoding)
def get_adinfo_list(file_name):
	adinfo_list=[]
	fp = codecs.open(file_name, encoding='gbk')
	#fp = open(file_name)
	while 1:
		line = fp.readline()
		if not line:
			break
		if line[0]=='#':
			continue
		field_list=line.strip().split('\t')
		if len(field_list)>=2:
			adinfo = adid.AdInfo()
			adinfo.key = field_list[0]
			#adinfo.category = field_list[1]
			#adinfo.item = field_list[2]
			#adinfo.type = field_list[3]
			#adinfo.pv = field_list[4]
			#adinfo.rank = field_list[5]
			adinfo.url = field_list[1]
			adinfo_list.append(adinfo)
	return adinfo_list
	
def dump_result():
	index=0
	if len(result_list) > 10000:
		index=10000
	else:
		index=len(result_list)
	for i in range(1,index):
		link= result_list.pop()
		print link


def get_pic_info(item_list):
	for item in item_list:
		pic_url = item.pic
		#print pic_url
		file = urllib2.urlopen(pic_url)
		tmpIm = cStringIO.StringIO(file.read())
		img = Image.open(tmpIm)
		img=img.resize((105,105))
		item.img=img
		time.sleep(1)


def output_result(dest_dir,item_list):
	top_list = item_list[0:4]
	os.mkdir(dest_dir+"/dir/")
	index = int(0)
	fp = open(dest_dir+'/dir/info', 'w')
	for item in top_list:
		pic_fname= "%s%d.jpg" % ("pic",index)
		fp.write('%s\3%s\3%s\3%s\3%s\n' % ( item.url,item.title.encode("utf-8"),item.summary.encode("utf-8"),item.price,pic_fname))
		item.img.save(dest_dir+"/dir/"+pic_fname)
		index +=1
	fp.close()
	index = int(0)
	os.mkdir(dest_dir+'/item');
	fp = open(dest_dir+'/item/info', 'w')
	for item in item_list:
		pic_fname= "%s%d.jpg" % ("pic",index)
		fp.write('%s\3%s\3%s\3%s\3%s\n' % ( item.url,item.title.encode("utf-8"),item.summary.encode("utf-8"),item.price,pic_fname))
		item.img.save(dest_dir+"/item/"+pic_fname)
		index +=1
	fp.close()

def get_html_content(ghost,url):
	try:
		page, extra_resources = ghost.open(url)
		return page.http_status,ghost.content
	except Exception,e:
		return 404,""


##################################
#main
##################################

adlist= get_adinfo_list(target_file)
print len(adlist)
#for ad in adlist:
#	print ad.url
#sys.exit() 
ghost = Ghost()
parser = suning.SuningParser()
index=0
for ad in adlist:
	result_list=[]
	spider_list=[]
	dest_dir = '%s/%d' % (root_dir,index)
	if not len(ad.url)>0:
		continue
	if not os.path.isdir(dest_dir): 
		os.mkdir(dest_dir)
	req = base.CrawlRequest()
	req.url=ad.url;

	fp = open(dest_dir + "/url",'w')
	fp.write(req.url+"\n")
	fp.close()

	log.info_log(("[begin crawl dir url %d]" % index)+ req.url)
	try:

		while (1):
			if req.retry_times ==0:
				req.error=1
				break
	
			try:
				res=urllib.urlopen(req.url).read()
			except Exception,e:
				req.retry_times -=1
				time.sleep(1)
				log.error_log("try again!"+req.url)
				continue

			if res:
				parser.dir_parse(res,result_list)
				break
			else:
				req.retry_times -=1
				time.sleep(1)
				log.error_log("try again!"+req.url)

		if req.error==0:
			#make dir result
			item_list =[]
			while( len(result_list)>0):
				crawl_req = result_list.pop()
				if crawl_req.retry_times ==0:
					log.error_log("[detail_url][crawl_error]"+crawl_req.url)
					continue
				#res=urllib.urlopen(url).read()
				status,res = get_html_content(ghost,crawl_req.url)
				if status ==200:
					#print res.encode("utf-8")
					item = commodity.Item()
					item.url = crawl_req.url
					parser.detail_parse(res,item)
					item_list.append(item)
	
				else:
					crawl_req.retry_times -=1
					result_list.append(crawl_req)
					time.sleep(1)
					log.error_log("[detail_url][try again]"+crawl_req.url)
	
			#make get pic info
			get_pic_info(item_list)
			output_result(dest_dir,item_list)
		else:
			log.error_log("[dir_url][crawl_error]"+req.url)		
	except Exception,e:
		print e
		log.error_log("[dir_url][crawl_error]"+req.url)
	index +=1


