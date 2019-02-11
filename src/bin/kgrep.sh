#!/bin/sh

egrep $* ${DIRVSTREE}/src/vstree/src/kurtz-basic/*.c\
         ${DIRVSTREE}/src/vstree/src/kurtz/*.c
