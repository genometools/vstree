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

for dirname in kurtz-basic kurtz kurtz.extra Vmengine
do
  cmd="cd ${dirname}"
  ${cmd}
  checkerror
  cmd="Mkincludeslib.sh"
  ${cmd}
  checkerror
  cmd="cd .."
  ${cmd}
  checkerror
done

for dirname in Mkvtree Vmatch Multimat Raremems Vmerstat fmmatch GeneCluster Unwords Minunique Mergesarr Smwa Swift Vnodes Suffixerator
do
  cmd="cd ${dirname}"
  ${cmd}
  checkerror
  cmd="Mkincludes.sh"
  ${cmd}
  checkerror
  cmd="cd .."
  ${cmd}
  checkerror
done
