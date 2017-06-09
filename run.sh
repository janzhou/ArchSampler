#!/bin/bash

# ./run.sh benchmark.exe 4 4096 a1-w1-s4096

[ -d STATS ] || mkdir STATS
sst ariel.py $1 $2 $3 $4 | tee STATS/$1-b$2-r$3-$4.output.txt
