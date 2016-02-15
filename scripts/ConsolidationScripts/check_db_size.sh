#!/bin/sh
dbSize=`du -s /opt/libd/var/lib/mysql/openvr |awk '{print $1}'`
xytSize=`du -s /opt/openvr/stored_minutiae |awk '{print $1}'`
echo $(($dbSize + $xytSize))
exit;
