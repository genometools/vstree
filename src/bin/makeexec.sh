#!/bin/sh
if test $# -ne 3
then
  echo "Usage: $0 <directory> <user> <group>"
  exit 1
fi
DIR=$1
THEUSER=$2
GROUP=$3

find ${DIR} -user ${THEUSER} -print -exec chgrp ${GROUP} '{}' \;

find ${DIR} -user ${THEUSER} -type d -print -exec chmod gu+rx '{}' \;

find ${DIR} -user ${THEUSER} -path '*.sh' -print -exec chmod gu+xr '{}' \;

find ${DIR} -user ${THEUSER} -path '*.pl' -print -exec chmod gu+xr '{}' \;

find ${DIR} -user ${THEUSER} -path '*.x' -print -exec chmod gu+xr '{}' \;

