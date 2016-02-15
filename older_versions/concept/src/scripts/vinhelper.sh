#!/bin/bash
targetFile=".nvi";
date +%s > $targetFile;
echo 0 >> $targetFile;
/sbin/ifconfig eth0 | awk -F: '/HWaddr/{print $3 $4 "-" $5 $6}' >> $targetFile;
