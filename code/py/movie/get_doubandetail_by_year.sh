#!/bin/sh
year=$1
url_dir="/data/wangwei/movie/spider/url/"
res_dir="/data/wangwei/movie/spider/result/douban/"
if [ ! -d ${res_dir}${year}/ ]
then
	mkdir ${res_dir}${year}
fi
nohup python detail.py ${url_dir}${year}/ ${res_dir}${year}/ > doubani_detial_${year}.log &


