#!/bin/sh

DIR_MINUTIAE="/opt/openvr/stored_minutiae"

DIR_RAMCACHE="/home/openvr/.ramcached"

DIR_MINUTIAE_SYM="/opt/openvr/stored_minutiae_symx"

rsync -au $DIR_MINUTIAE $DIR_MINUTIAE_SYM/




