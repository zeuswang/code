year=$1
year2=$2
url_dir="/data/wangwei/movie/spider/url/"
res_dir="/data/wangwei/movie/spider/result/douban/"
i=1989
while [ $i -le 2008 ]
do
	echo $i
	if [ ! -d ${res_dir}${i}/ ]
	then
		mkdir ${res_dir}${i}
	fi

	python detail.py ${url_dir}${i}/ ${res_dir}${i}/ 1 > doubani_detial_${i}.log 2>&1
	i=$((i+1))
done


