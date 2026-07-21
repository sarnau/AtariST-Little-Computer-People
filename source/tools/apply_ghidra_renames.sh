#!/usr/bin/env bash
# apply_ghidra_renames.sh -- one-shot Ghidra rename pipeline.
#
# Regenerates ~/ghidra_scripts/lcp_rename_map.tsv from
# source/tools/ghidra_globals_map.md, then triggers Ghidra's
# RenameLcpGlobals.java via the local HTTP server, and reports
# the outcome.
#
# Prereqs:
#   - Ghidra open with LCP.PRG loaded, HTTP server on :8089
#   - ~/ghidra_scripts/RenameLcpGlobals.java installed
#   - /tmp/ghidra_syms.txt fresh (regenerate via list_data_symbols.java
#     if the Ghidra project's symbol layout has changed)

set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
GHIDRA_URL=${GHIDRA_URL:-http://localhost:8089}

# 1. Regenerate the address<TAB>current<TAB>new TSV from the map.
echo "==> Regenerating TSV from map file"
python3 "$HERE/gen_ghidra_rename_tsv.py"

# 2. Sanity-check Ghidra is up.
if ! curl -sf "$GHIDRA_URL/run_script" \
       -X POST -H 'Content-Type: application/json' -d '{}' \
       >/dev/null 2>&1; then
    echo "ERROR: Ghidra HTTP not responding at $GHIDRA_URL." >&2
    echo "Open Ghidra with LCP.PRG loaded and try again." >&2
    exit 1
fi

# 3. Fire the rename script.
echo "==> Running RenameLcpGlobals.java in Ghidra"
resp=$(curl -sf "$GHIDRA_URL/run_script" \
            -X POST -H 'Content-Type: application/json' \
            -d '{"script_path":"RenameLcpGlobals.java"}' || {
        echo "ERROR: HTTP call failed" >&2; exit 1; })

echo
echo "$resp"

# 4. Extract the summary line and set exit code based on success.
summary=$(echo "$resp" | grep -E '^renamed=' | head -1 || true)
if [ -z "$summary" ]; then
    echo
    echo "ERROR: no `renamed=` summary line found in response" >&2
    exit 2
fi

# Non-zero if any name_drift or failed rows.
drift=$(echo "$summary" | sed -n 's/.*name_drift=\([0-9]*\).*/\1/p')
failed=$(echo "$summary" | sed -n 's/.*failed=\([0-9]*\).*/\1/p')
if [ "${drift:-0}" -gt 0 ] || [ "${failed:-0}" -gt 0 ]; then
    echo "WARN: drift=$drift failed=$failed -- inspect output above" >&2
    exit 3
fi

echo
echo "OK: $summary"
