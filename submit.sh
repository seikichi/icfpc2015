#!/bin/bash -e

# Usage: ./submit.sh problem_file_1.json problem_file_2.json ...

if [ ! -n "${API_TOKEN-}" ]; then
    echo 'please set API_TOKEN env. variable'
    exit -1
fi

if [ ! -n "${TEAM_ID-}" ]; then
    echo 'please set API_TOKEN env. variable'
    exit -1
fi

make

for problem in $@; do
    OUTPUT=$(./play_icfp2015 -f "$problem")
    curl --user :$API_TOKEN \
         -X POST \
         -H "Content-Type: application/json" \
         -d "$OUTPUT" \
         https://davar.icfpcontest.org/teams/$TEAM_ID/solutions ; 
done
