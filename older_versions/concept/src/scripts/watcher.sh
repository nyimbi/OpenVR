#!/bin/bash
process_name="geditx"
isRunning=`pidof $process_name`

while [ $isRunning == "" ]
do
#xterm &
#i=$[$i+1]
echo "not found"
sleep 1;
done

#if [ "$(pidof $process_name)" ] 
#then
  # process was found
#	echo "process found"
#else

#echo "process NOT found"
  # process not found
#fi



