#!/bin/bash

pwd=$(pwd)

scripts/download_movielens.sh

cd $pwd

sst ariel.py movie_count.exe 4 128
sst ariel.py movie_count.exe 8 128
sst ariel.py movie_count.exe 16 128
sst ariel.py movie_count.exe 32 128
sst ariel.py movie_count2.exe 4 128
sst ariel.py movie_count2.exe 8 128
sst ariel.py movie_count2.exe 16 128
sst ariel.py movie_count2.exe 32 128
sst ariel_worst.py movie_count.exe 4 128
sst ariel_worst.py movie_count.exe 8 128
sst ariel_worst.py movie_count.exe 16 128
sst ariel_worst.py movie_count.exe 32 128
sst ariel_worst.py movie_count2.exe 4 128
sst ariel_worst.py movie_count2.exe 8 128
sst ariel_worst.py movie_count2.exe 16 128
sst ariel_worst.py movie_count2.exe 32 128
