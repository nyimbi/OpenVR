#!/bin/bash

PASSWD="password"

echo "$PASSWD" | sudo -S fdisk -l | grep NTFS | cut -d " " -f1 | 
while read ntfsDrive; do
	echo "$PASSWD"|sudo -S ntfsfix $ntfsDrive 
done

