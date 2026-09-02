#!/usr/bin/env bash
# alcyon_build.sh -- drive Alcyon C 4.14 NATIVELY on macOS.
#
# Uses the native-macOS ports of cp68/c068/c168/as68 at ~/Hatari_C/hatari-c/bin/
# instead of Hatari, so each compile takes a fraction of a second
# instead of ~15 seconds.
#
# Environment:
#   ALCYON_BIN  directory holding cp68/c068/c168/as68
#               (default: $HOME/Hatari_C/hatari-c/bin)
#   ALCYON_INC  directory holding Alcyon system headers (osbind.h etc.)
#               (default: $HOME/Hatari_C/hatari-c/TOOLS/INCLUDE)
#   FILES       space-separated .c basenames to build (default: all)

set -euo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
# Toolchain: host builds of the cleaned-up Alcyon sources
# (~/Hatari_C/hatari-c/src, from Compiler/Alcyon/alcyon) -- see
# CLAUDE.md "Toolchain reconstruction (2026-09-01)".
ALCYON_BIN=${ALCYON_BIN:-$HOME/Hatari_C/hatari-c/bin}
ALCYON_INC=${ALCYON_INC:-$HOME/Hatari_C/hatari-c/TOOLS/INCLUDE}
OUT=$CSRC/build/alcyon
WORK=$OUT/work

mkdir -p "$OUT" "$WORK"

# STX unity units (tools/stx_units.txt): the default build compiles
# each unit and skips its constituents, reproducing LCP_STX's object
# partition so as68 emits its bsr call shapes.  FAITHFUL does the
# reverse -- LCP_ORG's partition is the port's own file list.
UNITS=""; PARTS=""
if [ -f "$CSRC/tools/stx_units.txt" ]; then
    while read -r unit rest; do
        case "$unit" in ''|'#'*) continue;; esac
        UNITS="$UNITS $unit"
        PARTS="$PARTS $rest"
    done < "$CSRC/tools/stx_units.txt"
fi
case " ${ALCYON_CPPFLAGS:-} " in
    *-DFAITHFUL*) SKIP="$UNITS" ;;   # units off, constituents on
    *)            SKIP="$PARTS" ;;   # units on, constituents off
esac

# Which files to build?
if [ -n "${FILES:-}" ]; then
    # Asking for a file that is a unity-unit constituent in the active
    # configuration must rebuild its UNIT instead -- otherwise the
    # constituent object reappears alongside the unit and the link
    # gets duplicate symbols.
    TO_BUILD=""
    for f in $FILES; do
        case " $SKIP " in *" $f "*)
            rm -f "$OUT/${f%.c}.o"
            if [ -f "$CSRC/tools/stx_units.txt" ]; then
                u=$(awk -v f="$f" '$1 !~ /^#/ { for (i=2;i<=NF;i++) if ($i==f) print $1 }' \
                    "$CSRC/tools/stx_units.txt")
                case " $TO_BUILD " in *" $u "*) ;; *) TO_BUILD="$TO_BUILD $u";; esac
            fi
            continue;;
        esac
        TO_BUILD="$TO_BUILD $f"
    done
else
    TO_BUILD=""
    for c in "$CSRC"/*.c; do
        base=$(basename "$c")
        [ "$base" = "savehost.c" ] && continue
        case " $SKIP " in *" $base "*)
            rm -f "$OUT/${base%.c}.o"   # never leave a stale object
            continue;;
        esac
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

    # vdiown.c: LCP_ORG keeps the hand-assembly vdi_go (ROM 0xd664 --
    # c168 cannot emit trap #2) INSIDE this object, so as68 shortens
    # the calls to bsr.  LCP_STX calls it with jsr, i.e. from another
    # object, so for that configuration vdi_go is assembled into
    # vdiown_a.s instead and nothing is injected here.
    if [ "$stem" = "vdiown" ] && \
       case " ${ALCYON_CPPFLAGS:-} " in *-DFAITHFUL*) true;; *) false;; esac
    then
        python3 - "$WORK/vdiown.s" <<'PYEOF'
import sys
p = sys.argv[1]
s = open(p).read()
s = s.replace('jsr _vdi_go', 'bsr _vdi_go')
inj = """.text
.globl _vdi_go
_vdi_go:
link a6,#-4
move.l #_vdipb,d1
moveq #115,d0
trap #2
unlk a6
rts
"""
s = s.replace('.text', inj, 1)
open(p, 'w').write(s)
PYEOF
    fi

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
