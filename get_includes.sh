#!/bin/bash

rm true_libraries.txt filter.txt libraries.txt
cat $1 | grep include > libraries.txt
IFS=$'\n'

cat libraries.txt | grep \" > filter.txt

for ligne in $(<"filter.txt")
do
  echo ${ligne:9} >> true_libraries.txt
done
exit
