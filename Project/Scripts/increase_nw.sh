#!/bin/bash

if [ $# != 1 ]
then
  echo "usage: $0 nw_max"
  exit 1
fi

nw_max=$1

# create file
output_file="../Statistics/completion_time_vs_nw.csv"
touch $output_file

# write in file
echo "nw,completion_time" > $output_file
for (( i=1; i <= $nw_max; i++ ))
do
  completion_time=9
  echo "$completion_time,$i" >> $output_file
done
