#!/bin/bash


if [ ! -n "$1" ]; then
	echo "1|Missing Arguments";
	exit;
fi


if [ ! -n "$2" ]; then
	echo "2|Missing Arguments";
	exit;
fi



dumpPath=$1
dbFilesToCopy=$2
filesToCheck=""
filesToCopy=""







missingFiles=""

for j in $(echo $dbFilesToCopy | tr "," "\n")
do
  # process
  #echo $j;
  filesToCopy+="$j "
  
   if [ ! -e "${dumpPath}/${j}" ];then
  	missingFiles+="\n"
	missingFiles+="${dumpPath}/${j}"
	
	
	
  fi
done

if [ -n "$missingFiles" ];then
	echo -e "3|Missing Files - $missingFiles";
	
	exit;
fi

cd $dumpPath

for i in $(echo $dbFilesToCopy | tr "," "\n"|grep "MYI")
do
  # process
  #filesToCheck+="$i "
	myisamchk --silent --fast $i 2>../dump_${i}.log
	isCorrupt=`cat ../dump_${i}.log`

	if [ -n "$isCorrupt" ];then 
		echo "$i table is corrupt. Fixing "
		#this table is corrupt, so start automatic repair
		fixResult=`myisamchk --force --safe-recover $i`
		
	fi
	
done


tmpOutput=`cd -`

echo "0| Dump OK!"
