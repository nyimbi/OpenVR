#!/bin/bash
hostName=`hostname`
password="password"


ipAddress=$1
lanCard=`ifconfig -a | awk '{ print $1 }' | grep 'th'|head -n 1`
`echo $password | sudo -S ifconfig $lanCard up`
`echo $password | sudo -S ifconfig $lanCard $ipAddress netmask 255.255.0.0`

echo $lanCard
ifconfig $lanCard|grep inet
