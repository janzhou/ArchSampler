#!/bin/bash

pwd=$(pwd)

scripts/download_amazon_movies.sh

cd $pwd

# Word to search for wordcount application
word=story

PCM_SIZE_MB=2048

for workload in 7 6
do
	for n_samples in 512 1024 2048 4096 8192
	do
		for n_banks in 4 8 16 32
		do
			let "n_rows = (PCM_SIZE_MB * 1024 * 1024) / (8192 * n_banks)"
			sst ariel.py benchmark.exe $n_banks $n_rows a0-w$workload-s$n_samples-W$word-p1 > /dev/null
			sst ariel.py benchmark.exe $n_banks $n_rows a1-w$workload-s$n_samples-W$word-p1 > /dev/null
			sst ariel.py benchmark.exe $n_banks $n_rows a2-w$workload-s$n_samples-W$word-p1 > /dev/null
			sst ariel.py benchmark.exe $n_banks $n_rows ca1-w$workload-s$n_samples-W$word-p1 > /dev/null

#			./benchmark.exe -b$n_banks -r$n_rows -s$n_samples -w$workload -W$word -p1		

			echo "---------------------------------------------------------------------------------------"
		done
	done
done
