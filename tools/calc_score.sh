#!/bin/bash

header=""
data=""

for i in 0 1 9 15 23 24; do
    score=$(./play_icfp2015 -f problems/problem_${i}.json -t 60 2>&1 \
                | grep '^best_score' \
                | sed -e 's/best_score: //' \
                | awk '{m+=$1} END{print int(m/NR);}')
    header="${header}problem${i},"
    data="${data}${score},"
done

echo $header
echo $data
