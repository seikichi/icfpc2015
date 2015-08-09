#!/bin/bash -e

if [ $# -eq 0 ]; then
    echo 'Usage: solve_and_submit_all.sh problem_file_1.json problem_file_2.json ...'
    exit 1
fi

if [ ! -n "${API_TOKEN-}" ]; then
    echo 'please set API_TOKEN env. variable'
    exit -1
fi

if [ ! -n "${TEAM_ID-}" ]; then
    echo 'please set API_TOKEN env. variable'
    exit -1
fi

make

DIR=$(dirname $0)

for problem in $@; do
    OUTPUT=$(${DIR}/../play_icfp2015 -f "$problem")

    curl --user :$API_TOKEN \
         -X POST \
         -H "Content-Type: application/json" \
         -d "$OUTPUT" \
         https://davar.icfpcontest.org/teams/$TEAM_ID/solutions
done
