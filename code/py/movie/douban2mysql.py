# coding=utf8
import MySQLdb
import sys
import os

target_dir=sys.argv[1]
db = MySQLdb.connect(host='localhost',user='root',passwd='fawcom',port=3306)
cursor = db.cursor()
#cursor.execute('create database if not exists movie')
db.select_db('movie')
cursor.execute("set names utf8;")  
#cursor.execute('create table minfo(id bigint unsigned NOT NULL AUTO_INCREMENT,cname varchar(20),ename varchar(20),actors varchar(20),type tinyint,date date,rate tinyint,PRIMARY KEY (id));')
#print dir(cursor);
#data = cursor.fetchall()
#print data
db.close
#  db.commit()

folders = os.listdir(target_dir)
for name in folders:
	print name
	for line in  open(target_dir +'/'+name,'r'):
		line = line.strip()
		rl = line.split(",")
		if len(rl) ==11:
			cname=rl[0]
			cname=cname.replace("\"","")
			ename=rl[1]
			ename=ename.replace("\"","")
			actors=rl[2]
			director=rl[3]
			writer=rl[4]
			writer=writer.replace(":","")
			writer=writer.replace(" ","")
			print writer 
			nation=rl[5]
			nation=nation.replace(" ","")
			print nation
			type=rl[6]
			date=rl[7]
			print date
			date_int = 0;
			if len(date)>4:
				try:
					date_int =int(date[0:4])
				except:  
					date_int = 0
			print date
			runtime=rl[8]
			print "xxxx"
			rate=0
			if len(rl[9])>0:
				rate=int(float(rl[9])*10)
				print rate
			votes=0
			if len(rl[10])>0:
				votes=int(rl[10])
				print votes

			sql='insert into minfo(cname,ename,actors,director,writer,nation,type,runtime,date,rate,votes) values("%s","%s","%s","%s","%s","%s","%s","%s",%d,%d,%d);' % (cname,ename,actors,director,writer,nation,type,runtime,date_int,rate,votes)
			print sql
			cursor.execute(sql)
			db.commit()
	
