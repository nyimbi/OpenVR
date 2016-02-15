#!/bin/bash
gcc cameraCheckAccess.c -o cameraCheckAccess -I/opt/include/opencv/ -L/opt/lib -lcv -lhighgui
