#!/usr/bin/env python3
import re
import sys

me = sys.argv[0]

if (len(sys.argv) != 2):
    print('Usage: {} src_file'.format(me))
    sys.exit(1)

src  = sys.argv[1]
skip = 0

# these functions are omitted by the C preprocessor when BENCH=1
RE = '.*(pr_info|ASSERT|printf|puts|dump_buf).*'

with open(src) as f:
    for line in f:
        if not skip and not re.match(RE, line):
            print(line, end='')
        else:
            print("{}: ignoring debug code line: {}".format(src, line), file=sys.stderr, end='')
        skip = line.endswith('\\\n')

sys.exit(0)
