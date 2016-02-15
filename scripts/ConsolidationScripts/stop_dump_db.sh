#!/bin/bash

PASS="0yVcBpu.Ce3g\$"

echo "Shutting down MySQL"
mysqladmin -u root --password=$PASS -P 30312 -h 127.0.0.1 shutdown




