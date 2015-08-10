#!/bin/bash -e

make

OUTPUT=$(AI=kichiai ./play_icfp2015 -f ./problems/problem_0.json 2>&1 1>/dev/null)

diff ./misc/kichiai_problem_0_stderr_20150810_1537.txt <(echo "$OUTPUT")
