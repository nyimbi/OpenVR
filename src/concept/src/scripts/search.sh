#!/bin/bash
result="";
cd stored_minutiae;
totalLis=`ls *.lis 2>&-|wc -l`
if [ "$totalLis" -le 0 ]
then
	exit;
fi

for x in `ls  *.lis `
do

 
 
#echo $x;
cmd="bozorth3 -q -T $1 -A minminutiae=$2 -A outfmt=s -p $3 -G $x";
#echo $cmd;
thisResult=`$cmd`
#echo $cmd;
#echo $thisResult;
if [ -n "$thisResult" ] && [ "$thisResult" -ge "10" ] 
then
	result+="$x ${thisResult} \n"
fi
done
echo -e $result;
#Please no trailing spaces!!!