#!/bin/bash

dumpDir="$1"
processedDumpPath="/openvr-station/processed-dumps"
mkdir -p $processedDumpPath

echo "Moving $dumpDir ..."

if [ -d "$dumpDir" ]; then
	mv "$dumpDir" "$processedDumpPath"
	echo "Dump Moved to Processed Dumps"
else 
	echo "This dump does not exist"
fi


