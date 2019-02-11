#!/bin/sh

egrep $* ${DIRVSTREE}/src/vstree/src/include/*.[ch]\
         ${DIRVSTREE}/src/vstree/src/include/*.pr
