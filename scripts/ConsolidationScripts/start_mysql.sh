#!/bin/bash
pidFile="/opt/libd/var/lib/mysql/*.pid";
rm -f $pidFile;
#start mysql
mysqld_safe --defaults-file="/opt/libd/my.cnf" &
sleep 3;
while [ ! -e $pidFile ]; do
# Sleep until mysqld starts successfully does exists/is created
sleep 1
done 
