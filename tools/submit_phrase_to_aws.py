#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import base64
import urllib.request
import json
import sys
import os
import warnings

if len(sys.argv) != 3:
    warnings.warn("Usage: send_phrase_to_aws.py problem.json phrase")
    exit(1)

problem = json.load(open(sys.argv[1]))
phrase = sys.argv[2]

result = [{'problemId': problem['id'], 'seed': seed, 'tag': 'phrase test', 'solution': phrase} for seed in problem['sourceSeeds']]

submit_url = 'http://ec2-52-69-136-236.ap-northeast-1.compute.amazonaws.com/ellatino/api/submits'
authorization = base64.b64encode('kmc:reikai'.encode('utf-8')).decode('ascii')
headers = {'Content-Type': 'application/json', 'Authorization': 'Basic ' + authorization}

req = urllib.request.Request(submit_url, data=json.dumps(result).encode('utf-8'), headers=headers)
response = urllib.request.urlopen(req)

result = json.loads(response.read().decode('utf8'))

new_tag = result['input']['new_tag']
print('new tag: {}'.format(new_tag))
print('http://ec2-52-69-136-236.ap-northeast-1.compute.amazonaws.com/ellatino/api/submits?q={}'.format(new_tag))

if result['input']['response'] != 'created':
    warnings.warn('submit failed ...')
    exit(1)

if os.environ.get('POLLING', '') != '1':
    exit(1)

print('polling start ...')

while True:
    url = 'http://ec2-52-69-136-236.ap-northeast-1.compute.amazonaws.com/ellatino/api/submits?q={}'.format(new_tag)
    authorization = base64.b64encode('kmc:reikai'.encode('utf-8')).decode('ascii')
    headers = {'Authorization': 'Basic ' + authorization}
    response = urllib.request.urlopen(urllib.request.Request(url, headers=headers))
    submit = json.loads(response.read().decode('utf8'))['submits'][0]
    if 'output' in submit:
        print(submit['output'])
        break
    time.sleep(10)
