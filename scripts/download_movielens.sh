#!/bin/bash

[ -d "data" ] || mkdir data
cd data
[ -f "ml-20m.zip" ] || wget http://files.grouplens.org/datasets/movielens/ml-20m.zip
[ -d "ml-20m" ] || unzip ml-20m.zip
