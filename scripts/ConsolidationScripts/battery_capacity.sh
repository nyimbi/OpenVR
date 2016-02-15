#!/bin/bash
remainingCapacity=`cat /proc/acpi/battery/BAT0/state | grep remaining | cut -d":" -f 2 | sed 's/[ ]//g' | sed 's/mAh//g'`

echo $remainingCapacity
