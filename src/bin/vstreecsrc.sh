#!/bin/sh
find ${DIRVSTREE}/src/vstree -type f -print | egrep -f ${DIRVSTREE}/src/vstree/src/bin/vstree.csrc
