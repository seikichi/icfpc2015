#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import datetime
import uuid
import json
import os
import warnings

import requests

from bson import json_util
from bottle import get, post, run, request, response, auth_basic, HTTPResponse
from pymongo import MongoClient

client = MongoClient(os.environ.get('MONGO_HOST', 'localhost'),
                     int(os.environ.get('MONGO_PORT', '27017')))
db = client.icfpc2015

TEAM_ID = os.environ.get('TEAM_ID', None)
API_TOKEN = os.environ.get('API_TOKEN', None)

if not TEAM_ID or not API_TOKEN:
    warnings.warn('please set TEAM_ID and API_TOKEN in environment variable.')
    exit(-1)

def error_response(message, status):
    return HTTPResponse({'message': message}, status=status)

def check_pass(username, password):
    return username == 'kmc' and password == 'reikai'

@post('/api/submits')
@auth_basic(check_pass)
def create_submit():
    try:
        data = json.loads(request.body.read().decode('utf8'))
    except:
        return error_response('invalid submit data, data must be a valid json', 400)

    if not isinstance(data, list):
        return error_response('invalid submit data, data must be an array.', 400)

    for d in data:
        if 'problemId' not in d:
            return error_response('invalid submit data, each data must contain problemId field.', 400)

    if len(set(d['problemId'] for d in data)) != 1:
        return error_response('invalid submit data, data must contains only one problemId.', 400)

    original_tags = []
    tag = str(uuid.uuid1())
    for d in data:
        original_tags.append(d.get('tag', None))
        d['tag'] = tag

    url = "https://davar.icfpcontest.org/teams/{0}/solutions".format(TEAM_ID)
    headers = {'Content-Type': 'application/json'}
    resp = requests.post(url, data=json.dumps(data), headers=headers, auth=('', API_TOKEN))
    text = resp.text

    input = {
        'data': data,
        'original_tags': original_tags,
        'new_tag': tag,
        'date': datetime.datetime.utcnow(),
        'response': text,
    }
    db.submits.insert_one({'input': input})
    response.content_type = 'application/json'
    return json_util.dumps({'input': input})

@get('/api/submits')
@auth_basic(check_pass)
def get_submit_list():
    queries = [q for q in (request.query.q or '').split('+') if q]
    keys = ['input.new_tag', 'input.original_tags']

    find_query = {}
    if queries:
        find_query = {'$and': [{'$or': [{key: re.compile('.*' + q + '.*')} for key in keys]} for q in queries]}
    print(queries, find_query)
    offset = request.query.offset or '0'
    if not offset.isdigit():
        return error_response('The offset parameter must be a signed integer.', 400)

    cursor = db.submits.find(find_query)

    limit = request.query.limit or str(cursor.count())
    if not limit.isdigit():
        return error_response('The limit parameter must be a signed integer.', 400)

    offset, limit = int(offset), int(limit)
    cursor = cursor.skip(offset).limit(limit)
    submits = list(cursor)

    response.content_type = 'application/json'
    return json_util.dumps({
        'submits': submits,
        'metadata': {
            'count': len(submits),
            'offset': offset,
            'limit': limit
        }
    })

run(host=os.environ.get('HOST', 'localhost'), port=int(os.environ.get('PORT', '8080')))
