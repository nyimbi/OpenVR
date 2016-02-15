#!/bin/bash
df -h|grep '/opt' | tr -d "\%" | awk -F " " '{ print "/opt " $1 " " $2 " " $3 " " $4 " " $5 " " $6 }'
df -h|grep 'sd.1' | tr -d "\%" | awk -F " " '{ print $1 " " $2 " " $3 " " $4 " " $5 " " $6 }'

