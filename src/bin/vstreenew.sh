#!/bin/sh
cd ${DIRVSTREE}/src/vstree
find . -type f -newer ${DIRVSTREE}/src/vstree.tgz -print | egrep -f ${DIRVSTREE}/src/vstree/src/bin/vstree.csrc
