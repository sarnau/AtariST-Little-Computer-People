#!/usr/bin/env bash
# alcyon_build.sh -- drive Alcyon C 4.14 NATIVELY on macOS.
#
# Uses the native-macOS ports of cp68/c068/c168/as68 at ~/hatari-c/bin/
# instead of Hatari, so each compile takes a fraction of a second
# instead of ~15 seconds.
#
# Environment:
#   ALCYON_BIN  directory holding cp68/c068/c168/as68
#               (default: $HOME/hatari-c/bin)
#   ALCYON_INC  directory holding Alcyon system headers (osbind.h etc.)
#               (default: $HOME/hatari-c/TOOLS/INCLUDE)
#   FILES       space-separated .c basenames to build (default: all)

set -euo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
ALCYON_BIN=${ALCYON_BIN:-$HOME/hatari-c/bin}
ALCYON_INC=${ALCYON_INC:-$HOME/hatari-c/TOOLS/INCLUDE}
OUT=$CSRC/build/alcyon
WORK=$OUT/work

mkdir -p "$OUT" "$WORK"

# Which files to build?
if [ -n "${FILES:-}" ]; then
    TO_BUILD="$FILES"
else
    TO_BUILD=""
    for c in "$CSRC"/*.c; do
        base=$(basename "$c")
        [ "$base" = "savehost.c" ] && continue
        TO_BUILD="$TO_BUILD $base"
    done
fi

built=0; missed=0; total=0
missed_list=""
for base in $TO_BUILD; do
    total=$((total + 1))
    src="$CSRC/$base"
    [ -f "$src" ] || { echo "  SKIP: $base"; missed=$((missed + 1)); continue; }
    stem="${base%.c}"

    # cp68: preprocess (extra flags via ALCYON_CPPFLAGS, e.g. -DFOO=1)
    "$ALCYON_BIN/cp68" -P -D__ALCYON__ ${ALCYON_CPPFLAGS:-} \
        -I "$CSRC/include" -I "$ALCYON_INC" \
        "$src" "$WORK/$stem.i" > /dev/null 2>&1 || {
        echo "  MISS $base (cp68)"; missed=$((missed + 1)); missed_list="$missed_list $base"; continue;
    }

    # c068: parse
    "$ALCYON_BIN/c068" "$WORK/$stem.i" \
        "$WORK/$stem.1" "$WORK/$stem.2" "$WORK/$stem.3" -f > /dev/null 2>&1 || {
        echo "  MISS $base (c068)"; missed=$((missed + 1)); missed_list="$missed_list $base"; continue;
    }

    # c168: generate assembly
    "$ALCYON_BIN/c168" "$WORK/$stem.1" "$WORK/$stem.2" "$WORK/$stem.s" > /dev/null 2>&1 || {
        echo "  MISS $base (c168)"; missed=$((missed + 1)); missed_list="$missed_list $base"; continue;
    }

    # Post-process .s:
    #  1. Truncate all _identifiers to 8 chars so as68 emits
    #     old-format single-entry symbol tables (native lo68/aln
    #     misinterpret c168's new-format multi-entry long-name
    #     encoding, seeing the continuation entries as separate
    #     unresolved symbols).
    #  2. Dedup L-labels (c168 emits duplicate L1: across data blocks).
    python3 -c "
import re
p = '$WORK/${stem}.s'
s = open(p).read()
# Truncate _identifiers to 8 chars total (7 C-name chars + underscore)
def trunc(m):
    name = m.group(0)
    return name[:8] if len(name) > 8 else name
s = re.sub(r'\b_[a-zA-Z_][a-zA-Z_0-9]*', trunc, s)
lines = s.split('\n')
seen = {}
out = []
for ln in lines:
    m = re.match(r'^(L\d+):(.*)\$', ln)
    if m:
        lbl = m.group(1)
        seen[lbl] = seen.get(lbl, 0) + 1
        if seen[lbl] > 1:
            out.append(f'{lbl}_{seen[lbl]}:{m.group(2)}')
            continue
    out.append(ln)
open(p, 'w').write('\n'.join(out))
"

    # as68: assemble.  Runs FROM the work dir so its temp files land there.
    (cd "$WORK" && "$ALCYON_BIN/as68" -l -u "$stem.s") > /dev/null 2>&1 || {
        echo "  MISS $base (as68)"; missed=$((missed + 1)); missed_list="$missed_list $base"; continue;
    }

    if [ -f "$WORK/$stem.o" ]; then
        cp "$WORK/$stem.o" "$OUT/$stem.o"
        built=$((built + 1))
    else
        missed=$((missed + 1))
        missed_list="$missed_list $base"
        echo "  MISS $base (no .o)"
    fi
done

echo "Alcyon build: $built OK, $missed missing"
if [ -n "$missed_list" ]; then echo "Missing:$missed_list"; fi
exit $([ "$missed" -eq 0 ] && echo 0 || echo 1)
