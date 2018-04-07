#!/bin/bash

if test -z "$1"
then 
  echo "bash: error missing one arg. Should precise the input c document"
  exit 
else 
  rm true_libraries.txt filter.txt libraries.txt 2> /dev/null
  cat $1 | grep include > libraries.txt
  IFS=$'\n'

  cat libraries.txt | grep \" > filter.txt

  for ligne in $(<"filter.txt")
  do
    echo ${ligne:9} >> true_libraries.txt
  done

  rm libraries.txt filter.txt
fi
exit
