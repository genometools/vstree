#!/bin/sh

LIBDIR=$DIRVSTREE/lib/${CONFIGGUESS}/32bit

extractsyms.sh ${LIBDIR}/libkurtz.a\
               ${LIBDIR}/libkurtz-basic.a\
               ${LIBDIR}/libmkvtree.a\
               ${LIBDIR}/libvmengine.a | sort -u > VMNAMES
