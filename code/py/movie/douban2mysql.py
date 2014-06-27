import MySQLdb
import sys
import os

target_dir=sys.argv[1]
db = MySQLdb.connect(host='localhost',user='root',passwd='fawcom',port=3306)
cursor = db.cursor()
#cursor.execute('create database if not exists movie')
db.select_db('movie')
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
		for r in rl:
			print r
	
