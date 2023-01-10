if $(echo $1 | grep svg > /dev/null); then
       	path=$(dirname "$1")
	file=$(basename "$1")
	#echo $1 | tee -a log.txt
	newName=$(echo "$file" | sed -e 's/_48//g' -e 's/_Light//g' -e 's/Res_//g' -e 's/Arch_//g' -e 's/AWS-//g' -e 's/Amazon-//g' -e 's/ //g' -e 's/ArchCategory//g')
	mv "${path}/${file}" "${path}/${newName}"
fi
