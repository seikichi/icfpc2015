#!/bin/bash -e

if [ $# -ne 1 ]; then
    echo 'Usage: submit_to_aws.sh solution_file.json'
    exit 1
fi

curl --user 'kmc:reikai' \
     -X POST \
     -H "Content-Type: application/json" \
     -d "@${1}" \
    'http://ec2-52-24-7-24.us-west-2.compute.amazonaws.com:8080/api/submits?q=8644'
