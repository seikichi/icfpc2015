#!/bin/bash

make

header=""
data=""

for i in {0..1}; do
    score=$(./play_icfp2015 -f problems/problem_${i}.json  2>&1 \
                | grep '^best_score' \
                | sed -e 's/best_score: //' \
                | awk '{m+=$1} END{print int(m/NR);}')
    header="${header}problem${i},"
    data="${data}${score},"
done

echo $header
echo $data
