#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import datetime
import uuid
import json
import os
import warnings

import requests

from bson import json_util
from bottle import get, post, run, request, response, auth_basic
from pymongo import MongoClient

client = MongoClient(os.environ.get('MONGO_HOST', 'localhost'),
                     int(os.environ.get('MONGO_PORT', '27017')))
db = client.icfpc2015

TEAM_ID = os.environ.get('TEAM_ID', None)
API_TOKEN = os.environ.get('API_TOKEN', None)

if not TEAM_ID or not API_TOKEN:
    warnings.warn('please set TEAM_ID and API_TOKEN in environment variable.')
    exit(-1)


def check_pass(username, password):
    return username == 'kmc' and password == 'reikai'

@post('/submits')
@auth_basic(check_pass)
def create_submit():
    data = json.loads(request.body.read().decode('utf8'))
    original_tags = []
    for d in data:
        original_tags.append(d['tag'])
        d['tag'] = str(uuid.uuid1())

    url = "https://davar.icfpcontest.org/teams/{0}/solutions".format(TEAM_ID)
    headers = {'Content-Type': 'application/json'}
    resp = requests.post(url, data=json.dumps(data), headers=headers, auth=('', API_TOKEN))
    text = resp.text

    for i in range(len(data)):
        db.submits.insert_one({
            'input': {
                'data': data[i],
                'original_tag': original_tags[i],
                'date': datetime.datetime.utcnow(),
                'response': text,
            }})

@get('/submits')
@auth_basic(check_pass)
def get_submit_list():
    response.content_type = 'application/json'
    return json_util.dumps(list(db.submits.find({})));

run(host=os.environ.get('HOST', 'localhost'), port=8080)
