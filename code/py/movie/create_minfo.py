import MySQLdb

db = MySQLdb.connect(host='localhost',user='root',passwd='fawcom',port=3306)
print "ok1"
cursor = db.cursor()
#cursor.execute('create database if not exists movie')
db.select_db('movie')
print "ok1"
cursor.execute('create table minfo(
id bigint unsigned NOT NULL AUTO_INCREMENT,
pic_url varchar(100) character set utf8,
cname varchar(100) character set utf8,
ename varchar(200) character set utf8,
actors text character set utf8, 
actor_links text character set utf8,
director varchar(200) character set utf8,
writer varchar(200) character set utf8,
nation varchar(100) character set utf8,
type varchar(255) character set utf8,
runtime varchar(50) character set utf8,
imdb_link varchar(100) character set utf8,
douban_link varchar(100) character set utf8,
comment_link varchar(100) character set utf8,
summary text character set utf8,
date smallint,
rate tinyint,
votes int,
PRIMARY KEY (id))
engine=innodb default charset=utf8 auto_increment=1;')
#print dir(cursor);
data = cursor.fetchall()
print data
db.close
#  db.commit()
