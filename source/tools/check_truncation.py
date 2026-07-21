#!/usr/bin/env python3
"""check_truncation.py -- Alcyon 8-char symbol-collision lint.

Alcyon's lo68 linker sees only the first 8 characters of every C-visible
identifier (7 name chars + leading underscore).  Post-process in
tools/alcyon_build.sh already truncates all `_ident` tokens in the .s
files.  If two distinct C names share the same 8-char prefix, the
truncation silently merges them at link time -- e.g. `vsf_color` and
`vsf_config` both become `_vsf_col`, and the linker picks one to
resolve calls to the other.

This tool scans every .s file under build/alcyon/work/ for exported
identifiers (`.globl _foo` and `_foo:` labels), computes the 8-char
prefix each one truncates to, and reports any prefix owned by more
than one distinct pre-truncation name.

Exit codes:
  0  no collisions
  1  collision(s) found
  2  no .s files found (build first)
"""

import glob
import os
import re
import sys
from collections import defaultdict

WORK = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'build', 'alcyon', 'work',
)

# Any `_ident` token -- catches definitions AND references.
# We want references too, because a caller with a long name that
# 8-char-truncates onto some *other* definition silently gets
# re-linked to the wrong symbol.
TOKEN_RE = re.compile(r'(?<![A-Za-z_0-9])(_[A-Za-z_][A-Za-z_0-9]*)')

# Alcyon-emitted internal labels we don't care about.
INTERNAL_RE = re.compile(r'^_(L\d+|LC\d+|LT\d+|LN\d+)$')


def collect():
    """Map 8-char truncation -> set of full names, plus a name -> files map."""
    prefix_to_names = defaultdict(set)
    name_to_files = defaultdict(set)

    files = sorted(glob.glob(os.path.join(WORK, '*.s')))
    if not files:
        return None, None, None

    for path in files:
        base = os.path.basename(path).replace('.s', '.c')
        with open(path) as f:
            for line in f:
                # Skip pure-comment lines (Alcyon uses ';' at column 0).
                if line.lstrip().startswith(';'):
                    continue
                for name in TOKEN_RE.findall(line):
                    if INTERNAL_RE.match(name):
                        continue
                    trunc = name[:8]
                    prefix_to_names[trunc].add(name)
                    name_to_files[name].add(base)

    return prefix_to_names, name_to_files, files


def main():
    prefix_to_names, name_to_files, files = collect()
    if prefix_to_names is None:
        print(f'SETUP: no .s files in {WORK} (run `make alcyon` first)',
              file=sys.stderr)
        sys.exit(2)

    collisions = {p: names for p, names in prefix_to_names.items()
                  if len(names) > 1}

    print(f'scanned {len(files)} .s files, '
          f'{sum(len(v) for v in prefix_to_names.values())} identifier defs, '
          f'{len(prefix_to_names)} distinct 8-char prefixes')

    if not collisions:
        print('no truncation collisions')
        sys.exit(0)

    print(f'COLLISIONS: {len(collisions)}')
    for prefix in sorted(collisions):
        names = sorted(collisions[prefix])
        print(f'  _{prefix[1:]:<7} <- ', end='')
        parts = []
        for n in names:
            src = ', '.join(sorted(name_to_files[n]))
            parts.append(f'{n} [{src}]')
        print(' + '.join(parts))
    sys.exit(1)


if __name__ == '__main__':
    main()
