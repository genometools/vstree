#!/bin/sh
for filename in rfbrute.x estmatch.x makefasta.x mstat.dbg.x mum.x oneFE.rand.x onePE.rand.x readdb.x
do
  if test ! -x ${HOME}/bin/${CONFIGGUESS}/${filename}
  then
    echo "warning: program \"${filename}\" does not exist or is not executable"
  fi
done
