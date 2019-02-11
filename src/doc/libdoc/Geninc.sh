#!/bin/sh
checkcmd() {
  if test $? -ne 0
  then
    echo "failure: ${cmd}"
    exit 1
  fi
}

if test $# -ne 1
then
  echo "$0 <texfile>"
  exit 1
fi
inputfile=${1}.tex
if test -f ${inputfile}
then
  echo "inputfile=${inputfile}"
else
  echo "$0: file \"${inputfile}\" does not exist"
  exit 1
fi

if test -f "${1}.specpath"
then
  specialpathfile="${1}.specpath"
  echo "specialpathfile=${specialpathfile}"
else
  specialpathfile=""
fi

C2LIT="c2lit.x -refnamefile Refnames"
exportmode="-export"
cmd="Extractfilenames.sh ${inputfile}"
${cmd} > filelist
checkcmd
allfiles=`cat filelist | tr '\n' ' '`

for filename in ${allfiles}
do
  filepath=""
  if test "${specialpathfile}" != ""
  then
    for pathname in `cat ${specialpathfile}`
    do
      if test -f ${pathname}/$filename
      then
        filepath=${pathname}/$filename
      fi 
    done
  fi
  if test "${filepath}" = ""
  then
    cmd="vpathkern.sh ${filename}"
    filepath=`${cmd}`
    checkcmd
  fi
  echo "filepath=${filepath}"
  ending=`echo ${filename} | tail -c 3`
  if test "${ending}" = ".c"
  then
    exportmode="-export"
    ## echo "############### ${filename} is a C-file"
  else
    exportmode=""
    ## echo "############### ${filename} is a something else"
  fi
  if test ! -f ${filepath}
  then
    echo "$0: file ${filepath} does not exist"
    exit 1
  fi
  if test ! -f ${filename}.inc
  then
    echo "generate \"${filename}.inc\" from \"${filepath}\""
    cmd="${C2LIT} ${exportmode} ${filepath}"
    ${cmd} > tmp
    checkcmd
    cmd="lit2alt.x -C tmp"
    ${cmd} > ${filename}.inc
    checkcmd
  else
    if test ${filepath} -nt ${filename}.inc
    then
      echo "update \"${filename}.inc\" from \"${filepath}\""
      cmd="${C2LIT} ${exportmode} ${filepath}"
      ${cmd} > tmp
      checkcmd
      cmd="lit2alt.x -C tmp"
      ${cmd} > ${filename}.inc
      checkcmd
    fi
  fi
done
rm -f tmp filelist
