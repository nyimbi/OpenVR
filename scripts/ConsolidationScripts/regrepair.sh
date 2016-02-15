#!/bin/bash
OPT_DIR="/opt/libd/var/lib/mysql/openvr"
cd $OPT_DIR
echo "This repair process may take some time. Please wait."
myisamchk --force --safe-recover -vvv registrations.MYI
 
