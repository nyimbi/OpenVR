#!/bin/sh
cd `dirname $0`
mysqladmin --protocol=tcp -u root --password=0yVcBpu.Ce3g\$ -P 30311 shutdown
sleep 2;
stillRunning=`./check_mysql.sh`

if [ ! $stillRunning ];then

	echo "1"
fi
