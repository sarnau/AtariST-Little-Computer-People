#!/usr/bin/env python3
"""ppbalance.py -- check #ifdef/#else/#endif balance in the port sources.

Campaign #2 gates nearly every recovered source shape behind
a conditional, and a scripted edit that wraps a line already
inside such a block leaves the outer `#else`/`#endif` stranded.
Alcyon's cp68 does not complain: it silently drops everything from
the unclosed `#ifdef` to EOF in the configuration that fails the
test, so the build still "succeeds" while whole functions vanish.

Run it after any batch edit:

    python3 source/tools/ppbalance.py
"""
import os
import sys

ROOTS = ('source', 'source/parts', 'source/include')


def check(path):
    stack, msgs = [], []
    for i, line in enumerate(open(path), 1):
        t = line.strip()
        if t.startswith(('#ifdef', '#ifndef', '#if ')):
            stack.append(i)
        elif t.startswith('#else') or t.startswith('#elif'):
            if not stack:
                msgs.append(f'stray {t.split()[0]} at line {i}')
        elif t.startswith('#endif'):
            if stack:
                stack.pop()
            else:
                msgs.append(f'stray #endif at line {i}')
    if stack:
        msgs.append('unclosed #ifdef at line ' +
                    ', '.join(str(i) for i in stack))
    return msgs


def main():
    base = os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__))))
    bad = 0
    for root in ROOTS:
        d = os.path.join(base, root)
        if not os.path.isdir(d):
            continue
        for f in sorted(os.listdir(d)):
            if not f.endswith(('.c', '.h')):
                continue
            msgs = check(os.path.join(d, f))
            if msgs:
                bad += 1
                print(f'{root}/{f}: ' + '; '.join(msgs))
    print(f'{bad} file(s) with unbalanced conditionals')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
