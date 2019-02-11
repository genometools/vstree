#!/bin/sh
selpath=Vmatch/SELECT
mustnumofheader=20
genmsg="$0: missing header files in ${selpath}"
if test -f ${selpath}/types.h
then
  numofheader=`ls ${selpath}/*.h | wc -l`
  if test ${numofheader} -ne ${mustnumofheader}
  then
    echo "${genmsg}: there must be ${mustnumofheader}"
    exit 1
    
    
  fi
else
  echo "${genmsg}: ${selpath}/types.h does not exist"
  exit 1
fi
echo "number of headers seems to be correct"
