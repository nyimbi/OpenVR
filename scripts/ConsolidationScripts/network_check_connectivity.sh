#!/bin/bash
ServerIP=$1
if nc -zv -w5 $ServerIP 22 <<< ” &> /dev/null
then
echo 0
else
echo 1
fi
exit;

