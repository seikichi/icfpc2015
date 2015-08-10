#!/bin/bash -e

header=""
data=""

for e in qwerty kichi qw; do
    # 標準エラー出力に best_score を出すので 2>&1
    output=$(EVALUATOR=${e} ./play_icfp2015 -f problems/problem_${ID}.json -t ${TIMEOUT:-60} 2>&1)
    echo "$output" 1>&2

    if [ $? -ne 0 ]; then
        output="best_score: -1"
    fi

    score=$(echo "$output" \
                   | grep '^best_score' \
                   | sed -e 's/best_score: //' \
                   | awk '{m+=$1} END{print int(m/NR);}')
    header="${header}${e}_problem${ID},"
    data="${data}${score},"
done

echo $header
echo $data
