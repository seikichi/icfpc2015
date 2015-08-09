#!/bin/bash

header=""
data=""

# 全問題を試す時間は無いので，良さそうなのを選んだ
for i in 0 1 9 15 23 24; do
    # 標準エラー出力に best_score を出すので 2>&1
    output=$(./play_icfp2015 -f problems/problem_${i}.json -t 60 2>&1)
    echo "$output" 1>&2

    if [ $? -ne 0 ]; then
        output="best_score: -1"
    fi

    score=$(echo "$output" \
                   | grep '^best_score' \
                   | sed -e 's/best_score: //' \
                   | awk '{m+=$1} END{print int(m/NR);}')
    header="${header}problem${i},"
    data="${data}${score},"
done

echo $header
echo $data
