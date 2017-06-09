#!/bin/bash

echo -n "Bank-0: "
grep "Bank: 0" $1 | wc -l

echo -n "Bank-1: "
grep "Bank: 1" $1 | wc -l

echo -n "Bank-2: "
grep "Bank: 2" $1 | wc -l

echo -n "Bank-3: "
grep "Bank: 3" $1 | wc -l
