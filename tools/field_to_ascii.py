#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import json

if len(sys.argv) != 2:
    print('Usage: field_to_ascii.py problem.json', file=sys.stderr)
    sys.exit(-1)

problem = json.load(open(sys.argv[1]))

height = problem['height']
width = problem['width']
filled = problem['filled']

for y in range(height):
    if y % 2 == 1:
        print(' ', end='')
    print('|', end='')
    for x in range(width):
        if {'x': x, 'y': y} in filled:
            print('<>', end='')
        else:
            print('  ', end='')
    print('|')

print('-' * (2 * width + 2))
