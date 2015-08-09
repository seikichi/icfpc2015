#!/bin/bash -e

if [ $# -ne 1 ]; then
    echo 'Usage: submit_to_official.sh solution_file.json'
    exit 1
fi

if [ ! -n "${API_TOKEN-}" ]; then
    echo 'please set API_TOKEN env. variable'
    exit 1
fi

if [ ! -n "${TEAM_ID-}" ]; then
    echo 'please set API_TOKEN env. variable'
    exit 1
fi

curl --user :$API_TOKEN \
     -X POST \
     -H "Content-Type: application/json" \
     -d "@${1}" \
     https://davar.icfpcontest.org/teams/$TEAM_ID/solutions
