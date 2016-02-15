#!/bin/bash
echo `df |grep 'sda1'|awk '{print $4}'`|tr -d "\%"

