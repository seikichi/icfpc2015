#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import datetime
import sys
import re
import json
import os
import time
import requests

from pymongo import MongoClient

client = MongoClient(os.environ.get('MONGO_HOST', 'localhost'),
                     int(os.environ.get('MONGO_PORT', '27017')))
db = client.icfpc2015
TEAM_ID = os.environ.get('TEAM_ID', None)
API_TOKEN = os.environ.get('API_TOKEN', None)

if not API_TOKEN or not TEAM_ID:
    print('Error: please set API_TOKEN and TEAM_ID to env. variable.')
    sys.exit(-1)

while True:
    try:
        print('downloading ...', file=sys.stderr)
        text = requests.get('https://davar.icfpcontest.org/rankings.js').content.decode('utf-8')
        print('done', file=sys.stderr)
        data = json.loads(re.sub(r'^var\s*data\s*=\s*', r'', text))
        for i, setting in enumerate(data['data']['settings']):
            for d in setting['rankings']:
                if d['teamId'] != int(TEAM_ID) or len(d['tags']) != 1:
                    continue
                tag = d['tags'][0]
                print("update {0}".format(tag), file=sys.stderr)
                output = {'data': d, 'date': datetime.datetime.utcnow()}
                db.submits.update({'input.new_tag': tag}, {'$set': {'output': output}}, upsert=False)
        print('sleeping ...', file=sys.stderr)
        time.sleep(20)
    except:
        print("Unexpected error:", sys.exc_info()[0], file=sys.stderr)
        time.sleep(20)
