#!/bin/bash -e

make

OUTPUT=$(AI=kichiai ./play_icfp2015 -f ./problems/problem_0.json 2>&1 1>/dev/null | grep -v '^best_score:')

diff ./misc/kichiai_problem_0_stderr_20150809_1552.txt <(echo "$OUTPUT")
