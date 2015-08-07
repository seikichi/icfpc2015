#!/usr/bin/env python3
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

units = problem['units']

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

print('')
print('-------')
for unit in units:
    members = unit['members']
    pivot = unit['pivot']
    maxy = 0
    for member in members:
        maxy = max(maxy, member['y'])
    for y in range(maxy + 1):
        if y % 2 == 1:
            print(' ', end='')
        for x in range(width):
            xy = {'x': x, 'y': y}
            if xy in members and xy == pivot:
                print('oo', end='')
            elif xy in members:
                print('<>', end='')
            elif xy == pivot:
                print('..', end='')
            else:
                print('  ', end='')
        print('')
    print('-------')
