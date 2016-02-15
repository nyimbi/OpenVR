#!/bin/bash
cd `dirname $0`
# echo "password" | sudo -S 
#lanCard=`echo "password" | sudo -S mii-tool |awk -F ':' '{ print $1 }'`
lanCard=`ifconfig -a | awk '{ print $1 }' | grep 'th'|head -n 1`

if [ ! $lanCard ];then
#echo "No Local Network Found"
exit;
fi

echo "password" | sudo -S dhclient $lanCard;
./network_current_ip.sh
exit;
