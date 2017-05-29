#!/bin/bash

[ -d "data" ] || mkdir data
cd data
[ -f "movies.txt" ] || wget https://snap.stanford.edu/data/movies.txt.gz
[ -f "movies.txt.gz" ] && gzip -d movies.txt.gz
