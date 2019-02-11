#!/bin/sh
checkerror() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  else
    echo "okay: ${cmd}"
  fi
}
transfiles="TransAnum  TransDNA TransDNAlowermask TransDNAX TransDNAX2 TransProt11  TransProt12 TransProt2 TransProt20 TransProt21   TransProt3 TransProt3B  TransProt4 TransProt4B  TransProt6  TransProt7  TransProt8 TransDNA_CBNP TransDNAall TransProtall"

for filename in ${transfiles}
do
  cmd="rsymmap.x ${filename}"
  ${cmd} > shit1
  checkerror

  cmd="rsymmap.x ${MKVTREESMAPDIR}/${filename}"
  ${cmd} > shit2
  checkerror

  cmd="cmp -s shit1 shit2"
  ${cmd}
  checkerror
done
