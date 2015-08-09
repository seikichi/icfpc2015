#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

while True:
    try:
        text = requests.get('https://davar.icfpcontest.org/rankings.js').text
        data = json.loads(re.sub(r'^var\s*data\s*=\s*', r'', text))
        for setting in data['data']['settings']:
            for d in setting['rankings']:
                if d['teamId'] != int(TEAM_ID) or len(d['tags']) != 1:
                    continue
                tag = d['tags'][0]
                db.submits.update({'input.data.tag': tag}, {'$set': {'output': d}}, upsert=False)
        time.sleep(60 * 1000)
    except:
        print("Unexpected error:", sys.exc_info()[0])
        time.sleep(60 * 1000)
