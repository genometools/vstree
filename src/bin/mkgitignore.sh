#!/bin/sh

find . -name '*.[ox]' | sed -e 's/\/[A-Za-z_0-9\.\-]*\.[ox]//' | sort -u > ~/OXdirs

for directory in `cat ~/OXdirs`
do
  printf "*.o\n*.x\n" > ${directory}/.gitignore
done
