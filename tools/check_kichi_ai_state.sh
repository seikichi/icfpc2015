#!/bin/bash -e

make

OUTPUT=$(AI=kichiai ./play_icfp2015 -f ./problems/problem_0.json 2>&1 1>/dev/null | grep -v  'problem_id')

diff ./misc/kichiai_problem_0_stderr_20150810_1537_in_ec2.txt <(echo "$OUTPUT")
