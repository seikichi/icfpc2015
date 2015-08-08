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
    print('Error: please set API_TOKEN and TEAM_ID to env. variable.')
    sys.exit(-1)

data = json.load(open(sys.argv[1]))
tags = set()

if len(set([d['problemId'] for d data])) != 1:
    print('Error: this program requires all problemIds are same')
    sys.exit(-1)

for i in range(len(data)):
    data[i]['tag'] = "{0} {1}".format(uuid.uuid1(), data[i]['tag'])
    tags.add(data[i]['tag'])

submit_url = "https://davar.icfpcontest.org/teams/{0}/solutions".format(TEAM_ID)
authorization = base64.b64encode((':' + API_TOKEN).encode('utf-8')).decode('ascii')
headers = {
    'Content-Type': 'application/json',
    'Authorization': 'Basic ' + authorization,
}

req = urllib.request.Request(submit_url, data=json.dumps(data).encode('utf-8'), headers=headers)
response = urllib.request.urlopen(req)
print(response.read().decode('utf8'))

while True:
    response = urllib.request.urlopen('https://davar.icfpcontest.org/rankings.js')
    data = json.loads(re.sub(r'^var\s*data\s*=\s*', r'', response.read().decode('utf8')))
    for setting in data['data']['settings']:
        for d in setting['rankings']:
            if d['teamId'] != 82 or set(d['tags']) != tags:
                continue
            print(d)
            sys.exit(0)
    time.sleep(10)
