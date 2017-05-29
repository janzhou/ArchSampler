#!/bin/bash

pwd=$(pwd)

scripts/download_amazon_movies.sh

cd $pwd

sst ariel.py amazon_movies_count.exe 4 32768
sst ariel.py amazon_movies_count.exe 8 16384
sst ariel.py amazon_movies_count.exe 16 8192
sst ariel.py amazon_movies_count.exe 32 4096
sst ariel.py amazon_movies_count2.exe 4 32768
sst ariel.py amazon_movies_count2.exe 8 16384
sst ariel.py amazon_movies_count2.exe 16 8192
sst ariel.py amazon_movies_count2.exe 32 4096
sst ariel_worst.py amazon_movies_count.exe 4 32768
sst ariel_worst.py amazon_movies_count.exe 8 16384
sst ariel_worst.py amazon_movies_count.exe 16 8192
sst ariel_worst.py amazon_movies_count.exe 32 4096
sst ariel_worst.py amazon_movies_count2.exe 4 32768
sst ariel_worst.py amazon_movies_count2.exe 8 16384
sst ariel_worst.py amazon_movies_count2.exe 16 8192
sst ariel_worst.py amazon_movies_count2.exe 32 4096
