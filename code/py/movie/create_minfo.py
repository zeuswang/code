import MySQLdb

db = MySQLdb.connect(host='localhost',user='root',passwd='fawcom',port=3306)
print "ok1"
cursor = db.cursor()
#cursor.execute('create database if not exists movie')
db.select_db('movie')
print "ok1"
cursor.execute('create table minfo(id bigint unsigned NOT NULL AUTO_INCREMENT,cname varchar(20),ename varchar(20),actors varchar(20),type tinyint,date date,rate tinyint,PRIMARY KEY (id));')
#print dir(cursor);
data = cursor.fetchall()
print data
db.close
#  db.commit()
