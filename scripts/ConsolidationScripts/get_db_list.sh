#!/bin/bash
#NOT USED
ls -l /opt/libd/var/lib/mysql/openvr/| awk '{print $5 "\t" $8}'
