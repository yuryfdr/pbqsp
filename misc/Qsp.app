#!/bin/sh
export LD_LIBRARY_PATH=/mnt/ext1/system/lib
/mnt/ext1/system/bin/Qspreal.app "$@" >/mnt/ext1/qsperr.txt 2>&1


