#!/bin/bash -e

export AI=submarineai

export TIMEOUT=30
./tools/calc_score.sh > submarineai_30_output.csv

export TIMEOUT=60
./tools/calc_score.sh > submarineai_60_output.csv

export TIMEOUT=30

export ID=0
./tools/calc_score_with_evaluator.sh > evalustor_0_output.csv

export ID=1
./tools/calc_score_with_evaluator.sh > evalustor_1_output.csv

export ID=24
./tools/calc_score_with_evaluator.sh > evalustor_24_output.csv
