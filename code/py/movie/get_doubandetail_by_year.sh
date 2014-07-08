year=$1
year2=$2
url_dir="/data/wangwei/movie/doubaninfo/dir_result/"
res_dir="/data/wangwei/movie/doubaninfo/detail_result/"
i=$year
while [ $i -le $year2 ]
do
	echo $i
	if [ ! -d ${res_dir}${i}/ ]
	then
		mkdir ${res_dir}${i}
	fi

	python detail.py ${url_dir}${i}/ ${res_dir}${i}/ 1 > ${res_dir}/${i}.log 2>&1
	i=$((i+1))
done


