#!/bin/sh

VMINP=Vmatchinput
mkdir -p ${VMINP}
for filename in $ATK $AT $SWK $SW $SWS $U8GB $U8
do
  if test -f "$filename"
  then
    echo "file $filename exists"
    cp ${filename} ${VMINP}/.
  else
    echo "file $filename does not exist"
    exit 1
  fi
done

cat << ENDOFECHO > ${VMINP}/setenv.tcshrc
setenv VMINP "/home/kwimmenauer/Vmatchinput"
setenv ATK "\${VMINP}/`basename ${ATK}`"
setenv AT "\${VMINP}/`basename ${AT}`"
setenv SWK "\${VMINP}/`basename ${SWK}`"
setenv SW "\${VMINP}/`basename ${SW}`"
setenv SWS "\${VMINP}/`basename ${SWS}`"
setenv U8GB "\${VMINP}/`basename ${U8GB}`"
ENDOFECHO
