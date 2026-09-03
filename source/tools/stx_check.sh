#!/usr/bin/env bash
# stx_check.sh -- campaign #2 inner loop.
#
# Rebuilds the given source files (default build), relinks LCP.PRG and
# the lcp_sym.68k side link, then runs verify_bytes against
# DATA/LCP_STX.PRG.  Extra
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

python3 "$CSRC/tools/ppbalance.py" | tail -1

if [ -n "$FILES_ARG" ]; then
    FILES="$FILES_ARG" "$CSRC/tools/alcyon_build.sh" >/dev/null
fi
"$CSRC/tools/alcyon_link.sh" >/dev/null

cd "$OUT"
# Reuse alcyon_link.sh's own object list verbatim -- if the symbol
# side-link disagrees with LCP.PRG about object order, every symbol
# extent comes out wrong and the sweep reports divergence that is not
# real.
sed 's/^lcp\.68k=/lcp_sym.68k=/' lcp_link.cmd > lcp_sym.cmd
~/Hatari_C/hatari-c/bin/link68 "[SYMBOLS,UNDEFINED,COMMAND[lcp_sym.cmd]]" >/dev/null 2>&1
cd "$CSRC/.."

python3 - "$NAMES" <<'EOF'
import importlib.util, sys, io, contextlib
names = set(sys.argv[1].split())
spec = importlib.util.spec_from_file_location(
    'vb', 'source/tools/verify_bytes.py')
vb = importlib.util.module_from_spec(spec)
spec.loader.exec_module(vb)
vb.ORIG = 'DATA/LCP_STX.PRG'
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
