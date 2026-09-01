#!/usr/bin/env bash
# stx_check.sh -- campaign #2 inner loop.
#
# Rebuilds the given source files (default build), relinks LCP.PRG and
# the lcp_sym.68k side link, then runs verify_bytes against
# DATA/LCP_STX.PRG with the kept-classification disabled.  Extra
# function names (without leading underscore) limit the report.
#
#   source/tools/stx_check.sh movement.c hs_posX
#   source/tools/stx_check.sh                     # full sweep summary

set -euo pipefail
CSRC=$(cd "$(dirname "$0")/.." && pwd)
OUT=$CSRC/build/alcyon

FILES_ARG=""
NAMES=""
for a in "$@"; do
    case "$a" in
        *.c) FILES_ARG="$FILES_ARG $a" ;;
        *)   NAMES="$NAMES $a" ;;
    esac
done

if [ -n "$FILES_ARG" ]; then
    FILES="$FILES_ARG" "$CSRC/tools/alcyon_build.sh" >/dev/null
fi
"$CSRC/tools/alcyon_link.sh" >/dev/null

cd "$OUT"
OBJS=$(find . -maxdepth 1 -name "*.o" ! -name "gemstart.o" ! -name "main.o" \
    ! -name "osbind.o" ! -name "crt0.o" ! -name "nofloat.o" \
    ! -name "vdilib.o" ! -name "vdilib_a.o" | sort | sed 's|^\./||')
~/hatari-c/bin/lo68 -r -o lcp_sym.68k \
    gemstart.o main.o $OBJS vdilib.o vdilib_a.o \
    vdibind.a aesbind.a osbind.o gemlib.a libf gemlib.a libf \
    >/dev/null 2>&1
cd "$CSRC/.."

python3 - "$NAMES" <<'EOF'
import importlib.util, sys, io, contextlib
names = set(sys.argv[1].split())
spec = importlib.util.spec_from_file_location(
    'vb', 'source/tools/verify_bytes.py')
vb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(vb)
vb.ORIG = 'DATA/LCP_STX.PRG'
vb.KEPT_PREFIXES = tuple()
vb.KEPT_NAMES = set()
sys.argv = ['verify_bytes.py', '-v'] + sorted(names)
buf = io.StringIO()
with contextlib.redirect_stdout(buf):
    try:
        vb.main()
    except SystemExit:
        pass
out = buf.getvalue()
if names:
    for l in out.splitlines():
        if any(n in l for n in names) or 'matched,' in l:
            print(l)
else:
    for l in out.splitlines():
        if 'matched,' in l:
            print(l)
EOF
