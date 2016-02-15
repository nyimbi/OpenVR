#!/bin/sh
x=0;
y=2;
enrollFile=$1;
c=$2;

bzT=$3;
bzM=$4;
#echo `ls -l`
absDir=`pwd`"/templates/"
tfile="${absDir}subject";
#echo $absDir;
#o=`cd /home/femi/qt4apps/OpenVRSystem/concept/templates`;
#echo `ls -l`


`rm -f ${tfile}.*`
#echo "nfiq ${absDir}${c}.pgm"
#echo "nfiq ${absDir}${c}.pgm"
nfiqResult=`nfiq ${absDir}${c}.pgm`
#echo $nfiqResult;
if [ $(($nfiqResult+0)) -gt $y ]; then
echo "-1";
exit;
fi

#echo "here";
#echo `pwd`
#echo "mindtct ${absDir}${c}.pgm $tfile"

`mindtct ${absDir}${c}.pgm $tfile`
#sleep 1.5;
sleep 1;
for  a in  2 3 4 5 6 7 8 9 10
do
if [ "$a" -gt "$c" ]; then 
break;
fi

b=$(( $a - 1 ));
#echo "$a $b";
#continue;
#`mindtct ../${c}.pgm $c`;
#echo 	"bozorth3 -q -T $2 -A minminutiae=$3 -A outfmt=s ../${b}.xyt ${c}.xyt"
#echo "bozorth3 -q -T 50 -A outfmt=s ${absDir}${b}.xyt ${tfile}.xyt";

result=`bozorth3 -q -T 24 -A outfmt=s ${absDir}${b}.xyt ${tfile}.xyt`;
#result=`bozorth3  -A outfmt=s ../${b}.xyt ${c}.xyt`;
#echo $result;
#echo $result;
if [ $(($result+0)) -gt $x ]; then
echo $result;
break;
fi
done
#Please no trailing spaces!!!
