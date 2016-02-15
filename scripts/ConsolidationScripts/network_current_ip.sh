#!/bin/sh
#sleep 3
result=`ifconfig  -a| grep 'inet addr:'| grep -v '127.0.0.1' | grep -i Bcast |cut -d: -f2 | awk '{ print $1}'|head -n 1`
echo $result
exit
