#!/bin/bash -e

for t in 60 30 10 5; do
    ulimit -t $t
    ./play_icfp2015 -f ./problems/problem_24.json -t $t || exit 1
done
