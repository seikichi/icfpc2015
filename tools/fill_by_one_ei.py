#!/usr/bin/env python
# -*- coding: utf-8 -*-

u'''
与えられた問題のフィールドに対して，1セルのみのユニットと仮定した上で

1. ユニットを左端に移動
2. ユニットを下端まで移動
3. ユニットを動かせるだけ右に移動

を繰り返すコマンドを生成する．
'''

import sys
import json

if len(sys.argv) != 2:
    print('Usage: field_to_ascii.py problem.json', file=sys.stderr)
    sys.exit(-1)

W, E, SW, SE = '!', 'e', 'i', ' '

problem = json.load(open(sys.argv[1]))

height = problem['height']
width = problem['width']
source_seeds = problem['sourceSeeds']
source_length = problem['sourceLength']

output = []

for seed in source_seeds:
    moved = 0
    commands = []
    for i in range(source_length):
        for j in range(width):
            commands.extend([W] * ((width - 1) // 2))
            commands.extend(([E, SW, W, SE] * height)[:height - 1])
            commands.extend([E] * (width - j))

    output.append({
        'problemId': problem['id'],
        'seed': seed,
        'tag': 'poyo}',
        'solution': ''.join(commands),
    })

print(json.dumps(output))
