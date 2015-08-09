#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
import json
import os
import re
import sys
import time
import urllib.request
import uuid

TEAM_ID = os.environ['TEAM_ID']
API_TOKEN = os.environ['API_TOKEN']

if len(sys.argv) != 2:
    print('Usage: submit.py output.json', file=sys.stderr)
    sys.exit(-1)

if not API_TOKEN or not TEAM_ID:
    print('Error: please set API_TOKEN and TEAM_ID to env. variable.', file=sys.stderr)
    sys.exit(-1)

data = json.load(open(sys.argv[1]))

if len(data) != 1:
    print('Error: this program can\'t handle multiple output.', file=sys.stderr)
    sys.exit(-1)

def test(problem_id, seed, solution):
    tag = str(uuid.uuid1())
    print(tag)
    submit_url = "https://davar.icfpcontest.org/teams/{0}/solutions".format(TEAM_ID)
    authorization = base64.b64encode((':' + API_TOKEN).encode('utf-8')).decode('ascii')
    headers = {
        'Content-Type': 'application/json',
        'Authorization': 'Basic ' + authorization,
    }

    data = json.dumps([{
        'problemId': problem_id,
        'seed': seed,
        'tag': tag,
        'solution': solution
    }]).encode('utf-8')
    req = urllib.request.Request(submit_url, data=data, headers=headers)
    response = urllib.request.urlopen(req)
    print(response.read().decode('utf8'))

    while True:
        response = urllib.request.urlopen('https://davar.icfpcontest.org/rankings.js')
        data = json.loads(re.sub(r'^var\s*data\s*=\s*', r'', response.read().decode('utf8')))
        for setting in data['data']['settings']:
            for d in setting['rankings']:
                if d['teamId'] != int(TEAM_ID) or d['tags'] != [tag]:
                    continue
                return d['score']
            time.sleep(10)

solution = data[0]['solution']
print(solution)

# solution[:left] is valid
# solution[:right] is invalid
# left, right = 0, len(solution)
left, right = 0, 127

expected_scores = [
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    5,5,5,5,5,5,5,5,5,5,5,5,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,11,11,11,11,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,13,13,13,13,13,
    13,13,13,13,13,13,13,15,15,15,15,15,15,15,15,15,15,15,15,17,17,17,17,
    17,17,17,17,17,17,17,120,120,120,120,120,120,120,120,120,
]
print(len(expected_scores))

while right - left > 1:
    mid = (left + right) // 2
    print("try solution[:{0}]...".format(mid))
    score = test(data[0]['problemId'], data[0]['seed'], solution[:mid])
    expected_score = expected_scores[mid - 1]
    if score == expected_score:
        print("=> valid (score = {0}, expected = {1})".format(score, expected_score))
        left = mid
    else:
        print("=> invalid (score = {0}, expected = {1})".format(score, expected_score))
        right = mid

print(solution[:right])
