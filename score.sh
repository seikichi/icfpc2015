#!/bin/bash -e

if [ $# -ne 1 ]; then
    echo 'Usage: score.sh description'
    exit 1
fi

make

DESCRIPTION=$1
mkdir -p score_logs/${DESCRIPTION}

for i in {0..23}; do
    AI=lightningai ./ella6 -f problems/problem_${i}.json \
      2> score_logs/${DESCRIPTION}/${i}.txt
done
