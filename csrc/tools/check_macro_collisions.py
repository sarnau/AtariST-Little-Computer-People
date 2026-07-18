#!/usr/bin/env python3
"""check_macro_collisions.py -- warn about #define macros whose names would
collide under Alcyon C 4.14's short macro-name lookup table.

Alcyon's preprocessor keys macros by only the first N characters of the
name.  Two macros whose first N chars are identical end up resolving to
whichever one was #define'd LATER, silently -- the shorter/earlier one
returns the wrong value in code with no compiler warning.

Empirically we've seen:
  STATE_STAIR_TOP_FRAME_0 (22) collide with STATE_STAIR_TOP_FRAME_3_STEP
  STATE_STAIR_BTM_FRAME_0 (22) collide with STATE_STAIR_BTM_FRAME_3
so the effective limit is <= 22 chars.  STATE_WALK_FRAME_0 (18) does NOT
collide with STATE_WALK_FRAME_1 (18), so the limit is >= 18.  This script
defaults to 22 (fewer false positives; only real known-broken cases).

Usage:
  python3 tools/check_macro_collisions.py            # limit=22
  LIMIT=20 python3 tools/check_macro_collisions.py   # more paranoid

Exit codes:
  0   no colliding groups whose earlier-defined members are used
  1   at least one macro is likely mis-compiled
"""

import os, re, subprocess, sys

LIMIT = int(os.environ.get('LIMIT', '19'))
SRC   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def read_defines():
    """Return dict {name: value_string} of every #define in include/*.h."""
    defs = {}
    for root, _, files in os.walk(os.path.join(SRC, 'include')):
        for f in files:
            if not f.endswith('.h'):
                continue
            path = os.path.join(root, f)
            with open(path) as fh:
                for line in fh:
                    m = re.match(r'#define\s+(\w+)\s+(\S+)', line)
                    if m:
                        defs[m.group(1)] = m.group(2)
    return defs

def find_usages(name):
    """Return list of basenames of .c files using `name` as a real reference —
    not just mentioned inside a /* ... */ comment (which is how the fixed
    call-sites annotate the numeric literal they replaced the macro with)."""
    r = subprocess.run(
        ['grep', '-rln', '--include=*.c', '-w', name, SRC],
        capture_output=True, text=True
    )
    files = []
    for path in r.stdout.strip().split('\n'):
        if not path:
            continue
        # Strip block comments, then look for the name as a whole word.
        text = open(path).read()
        stripped = re.sub(r'/\*.*?\*/', ' ', text, flags=re.DOTALL)
        if re.search(r'\b' + re.escape(name) + r'\b', stripped):
            files.append(os.path.basename(path))
    return files

def main():
    defs = read_defines()
    # Group by first LIMIT chars.  Groups with >1 member = collision.
    groups = {}
    for name in defs:
        groups.setdefault(name[:LIMIT], []).append(name)
    colls = {k: v for k, v in groups.items() if len(v) > 1}

    problems = []
    for key, names in sorted(colls.items()):
        # Preserve definition order — last-defined wins in Alcyon.
        winner = names[-1]
        winner_val = defs[winner]
        for loser in names[:-1]:
            if defs[loser] == winner_val:
                continue  # same value -- silent but harmless
            users = find_usages(loser)
            if users:
                problems.append((loser, defs[loser], winner, winner_val, users))

    if not problems:
        print(f'clean at limit={LIMIT}: no colliding macros in use')
        return 0

    print(f'FAIL at limit={LIMIT}: {len(problems)} broken macro(s) in use:')
    for name, val, winner, wval, users in problems:
        print(f'  {name} = {val}  → compiles as {winner} = {wval}')
        print(f'    used in: {", ".join(users)}')
    print()
    print('Replace each use with the numeric literal (see walk.c for the')
    print('pattern), or rename the macros in include/enums.h to be unique')
    print(f'within {LIMIT} chars.')
    return 1

if __name__ == '__main__':
    sys.exit(main())
