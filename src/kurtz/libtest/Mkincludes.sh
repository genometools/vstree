#!/bin/sh
echo "make Filegoals.mf"
ls *.c | mkfilegoals.pl prepro splint > Filegoals.mf
