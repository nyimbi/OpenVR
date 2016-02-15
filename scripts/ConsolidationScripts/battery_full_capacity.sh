#!/bin/bash

lastFullCapacity=`cat /proc/acpi/battery/BAT0/info | grep last | cut -d":" -f 2 | sed 's/[ ]//g' | sed 's/mAh//g'`
echo $lastFullCapacity
