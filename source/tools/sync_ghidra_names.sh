#!/usr/bin/env bash
# sync_ghidra_names.sh -- push the port's names into the Ghidra project.
#
# Replaces apply_ghidra_renames.sh, which POSTs to a Ghidra HTTP server
# on :8089 and needs RenameLcpGlobals.java and list_data_symbols.java in
# ~/ghidra_scripts.  None of those are installed and the endpoint does
# not answer, so this drives analyzeHeadless instead -- no server, no
# GUI, and it works on a closed project.
#
#   source/tools/sync_ghidra_names.sh          # push, then verify
#   source/tools/sync_ghidra_names.sh verify   # verify only, read-only
#
# Ghidra must NOT be running: it holds an exclusive lock on the project.
# The script refuses rather than fighting over it, because clearing a
# live lock is how a database gets corrupted.
#
# Addresses: Ghidra address = link address + 0x10000.  For BSS symbols
# the link address from lcp_sym.68k is lo68's, NOT the reference's --
# use tools/stx_bss_layout.tsv, which is what the remap places them at.
set -euo pipefail

REPO=$(cd "$(dirname "$0")/../.." && pwd)
GHIDRA=${GHIDRA_HOME:-/Applications/ghidra_12.1.2_PUBLIC}
SCRIPTS=${GHIDRA_SCRIPTS:-$HOME/ghidra_scripts}
MODE=${1:-sync}

if pgrep -f 'ghidra\.GhidraRun' >/dev/null 2>&1; then
    echo "ERROR: Ghidra is running -- close it first (it locks the project)." >&2
    exit 1
fi
[ -x "$GHIDRA/support/analyzeHeadless" ] || {
    echo "ERROR: no analyzeHeadless at $GHIDRA (set GHIDRA_HOME)." >&2; exit 1; }

cp "$REPO/source/tools/ghidra/LcpSyncNames.java" \
   "$REPO/source/tools/ghidra/LcpVerifyNames.java" "$SCRIPTS/"

case "$MODE" in
  verify) script=LcpVerifyNames.java; extra=(-readOnly) ;;
  sync)   script=LcpSyncNames.java;   extra=() ;;
  *) echo "usage: $0 [sync|verify]" >&2; exit 2 ;;
esac

"$GHIDRA/support/analyzeHeadless" "$REPO" LCP -process -noanalysis \
    ${extra[@]+"${extra[@]}"} -scriptPath "$SCRIPTS" -postScript "$script" 2>&1 |
    grep -E 'SYNC|VERIFY|ERROR' | sed 's/^INFO  [^>]*> //'
