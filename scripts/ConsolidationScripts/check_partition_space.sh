#!/bin/bash
df $1 |tail -n 1 |awk '{print $4}'
