#!/bin/sh

BBSHOME=/home/bbs/bbshome
DATESTRING=`date +%Y年%m月%d日`
DATAFILE=$BBSHOME/usies
OBJFILE=$BBSHOME/tmp/bbs.listkill

echo "[1;33m本日随风飘逝的ID:[m "  > $OBJFILE
grep KILL $DATAFILE | grep $DATESTRING | cut -f 6 -d "x" >> $OBJFILE
