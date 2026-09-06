#!/usr/bin/env bash
#
# run_all.sh -- the whole regression suite, in the order that works.
#
# Everything here was run by hand before this existed, and the ordering
# is the reason it exists: the byte-identity checks need the SHIPPED
# build, the runtime tests need a GATED one (-DSKIP_TITLE -DSKIP_COPYPROT,
# because the copy protection never passes under an emulator here), and
# leaving the wrong one in build/alcyon afterwards makes the next run
# lie.  A gated binary passes prg_diff never; a shipped binary parks the
# resident in cs_mvIn's wave loop and every runtime test fails 0/11 with
# no hint why.  This script always restores the shipped build, including
# on failure.
#
#   source/tools/run_all.sh            everything (~3.5 min measured)
#   source/tools/run_all.sh --quick    build + host only (~10 s)
#
# Env: HATARI, TOS_IMG, GAME_DIR pass through to the runtime tests;
#      SKIP_EMU=1 is the same as --quick.
#
# Exit: 0 all green, 1 something failed, 2 setup error.

set -uo pipefail

CSRC=$(cd "$(dirname "$0")/.." && pwd)
REPO=$(cd "$CSRC/.." && pwd)
REF=${LCP_REF:-DATA/LCP_STX.PRG}
QUICK=0
[ "${1:-}" = "--quick" ] && QUICK=1
[ -n "${SKIP_EMU:-}" ] && QUICK=1

pass=0; fail=0; skip=0; results=""
t0=$(date +%s)

ok()   { pass=$((pass+1)); printf '  PASS  %s\n' "$1"; results+=$'\n'"  PASS  $1"; }
bad()  { fail=$((fail+1)); printf '  FAIL  %s -- %s\n' "$1" "$2"
         results+=$'\n'"  FAIL  $1 -- $2"; }
skp()  { skip=$((skip+1)); printf '  SKIP  %s (%s)\n' "$1" "$2"
         results+=$'\n'"  SKIP  $1 ($2)"; }
phase(){ printf '\n=== %s ===\n' "$1"; }

# The shipped build is the resting state.  Restore it whatever happens,
# so a failed run does not leave a gated binary behind to confuse the
# next one.
restore_shipped() {
    printf '\n--- restoring the shipped build ---\n'
    rm -rf "$CSRC/build/alcyon"
    if "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 &&
       "$CSRC/tools/alcyon_link.sh"  >/dev/null 2>&1 &&
       cmp -s "$CSRC/build/alcyon/LCP.PRG" "$REPO/$REF"; then
        echo "    shipped build restored, byte-identical"
    else
        echo "    WARNING: could not restore a byte-identical shipped build" >&2
    fi
}
trap 'pkill -x hatari 2>/dev/null; restore_shipped' EXIT

cd "$REPO" || { echo "SETUP: cannot cd to $REPO" >&2; exit 2; }
[ -f "$REF" ] || { echo "SETUP: reference $REF missing" >&2; exit 2; }

# ---------------------------------------------------------------- 1/3
phase "1/3  shipped build -- byte identity and audits"
rm -rf "$CSRC/build/alcyon"
if "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 &&
   "$CSRC/tools/alcyon_link.sh"  >/dev/null 2>&1; then
    ok "shipped build compiles and links"
else
    bad "shipped build" "alcyon_build/alcyon_link failed"
fi

if cmp -s "$CSRC/build/alcyon/LCP.PRG" "$REF"; then
    ok "byte-identical to $REF  (md5 $(md5 -q "$CSRC/build/alcyon/LCP.PRG"))"
else
    bad "byte identity" "cmp differs from $REF"
fi

out=$(LCP_REF="$REF" python3 "$CSRC/tools/prg_diff.py" 2>&1)
echo "$out" | grep -q 'BYTE-IDENTICAL' \
    && ok "prg_diff" \
    || bad "prg_diff" "$(echo "$out" | tail -3 | tr '\n' ' ')"

# reloc_audit: every category must be 0 except E, which is scrbufA's
# inferred base and is documented as undecidable.
# Take the first number after the LAST ": " -- category D ends
# "0 of 283", so a plain ': 0$' test reports a false failure.
out=$(python3 "$CSRC/tools/reloc_audit.py" 2>&1)
nonzero() { awk -F': ' '{split($NF,a," "); if (a[1]+0 != 0) print}'; }
bad_rows=$(echo "$out" | grep -E '^[ABCDF]\.' | nonzero || true)
mm=$(echo "$out" | grep -E '^segment' | nonzero || true)
if [ -z "$bad_rows" ] && [ -z "$mm" ]; then
    ok "reloc_audit  (E=$(echo "$out" | sed -n 's/^E\..*: //p') expected: scrbufA)"
else
    bad "reloc_audit" "$(echo "$bad_rows$mm" | tr '\n' ' ')"
fi

out=$(bash "$CSRC/tools/stx_check.sh" 2>&1 | tail -2)
echo "$out" | grep -q '0 divergent' \
    && ok "stx_check  ($(echo "$out" | grep -o '[0-9]* matched'))" \
    || bad "stx_check" "$(echo "$out" | tr '\n' ' ')"

out=$(python3 "$CSRC/tools/ppbalance.py" $(ls "$CSRC"/*.c "$CSRC"/parts/*.c \
        "$CSRC"/include/*.h 2>/dev/null) 2>&1 | tail -1)
echo "$out" | grep -q '^0 file' \
    && ok "ppbalance  (no unbalanced conditionals)" \
    || bad "ppbalance" "$out"

# ---------------------------------------------------------------- 2/3
phase "2/3  host build and unit tests"
if (cd "$CSRC" && make clean >/dev/null 2>&1; make >/dev/null 2>&1); then
    ok "host build (every .c through clang)"
else
    bad "host build" "make failed -- run 'cd source && make' to see it"
fi
if (cd "$CSRC" && make linktest >/dev/null 2>&1); then
    ok "host linktest"
else
    bad "host linktest" "make linktest failed"
fi
out=$(cd "$CSRC" && make test 2>&1)
echo "$out" | grep -q 'all tests OK' \
    && ok "host unit tests  ($(echo "$out" | grep -c 'RUN ') binaries, all OK)" \
    || bad "host unit tests" "$(echo "$out" | grep -iE 'fail|error' | head -2 | tr '\n' ' ')"

# ---------------------------------------------------------------- 3/3
phase "3/3  runtime tests on the gated build"
if [ "$QUICK" = "1" ]; then
    skp "runtime tests" "--quick"
else
    HAT=${HATARI:-$HOME/Downloads/Hatari/hatari/build/src/hatari}
    TOSI=${TOS_IMG:-/Users/sarnau/Desktop/Retro/Atari ST/Atari TOS Images/TOS104US.ROM}
    if [ ! -x "$HAT" ] || [ ! -f "$TOSI" ]; then
        skp "runtime tests" "no Hatari at $HAT or no TOS ROM"
    else
        # Build the gated configuration ONCE; the tests reuse it via
        # NO_REBUILD, which is what keeps this to one link instead of four.
        rm -rf "$CSRC/build/alcyon"
        if ALCYON_CPPFLAGS="-DSKIP_TITLE=1 -DSKIP_COPYPROT=1" \
             "$CSRC/tools/alcyon_build.sh" >/dev/null 2>&1 &&
           "$CSRC/tools/alcyon_link.sh" >/dev/null 2>&1; then
            ok "gated build (-DSKIP_TITLE -DSKIP_COPYPROT)"

            for t in test_keyboard test_actions test_saveload; do
                pkill -x hatari 2>/dev/null; sleep 1
                out=$(NO_REBUILD=1 "$CSRC/tools/$t.sh" 2>&1)
                line=$(echo "$out" | grep -E '^ *passed [0-9]+' | tail -1)
                if echo "$out" | grep -qE 'failed 0$'; then
                    ok "$t  ($line)"
                else
                    bad "$t" "${line:-no summary}; $(echo "$out" | grep -c FAIL) FAIL line(s)"
                fi
            done

            # test_longrun_stable.sh invokes a BARE `hatari`, and there
            # is none on PATH here -- the build lives under ~/Downloads.
            # Give it a shim rather than edit the script.
            shim=$(mktemp -d -t lcp_hatari_shim)
            ln -sf "$HAT" "$shim/hatari"
            pkill -x hatari 2>/dev/null; sleep 1
            out=$(PATH="$shim:$PATH" NO_REBUILD=1 \
                  "$CSRC/tools/test_longrun_stable.sh" 2>&1)
            rm -rf "$shim"
            if echo "$out" | grep -q 'VERDICT: *STABLE'; then
                ok "test_longrun_stable  ($(echo "$out" | grep -o 'PSNR.*dB' | head -1))"
            else
                bad "test_longrun_stable" "$(echo "$out" | grep -E 'VERDICT|SETUP' | tr '\n' ' ')"
            fi
        else
            bad "gated build" "alcyon_build/alcyon_link failed with the SKIP_* flags"
        fi
    fi
fi

# ------------------------------------------------------------- summary
printf '\n================ SUMMARY ================\n'
echo -e "${results# }"
printf '\n  passed %d, failed %d, skipped %d, %d s elapsed\n' \
       "$pass" "$fail" "$skip" "$(( $(date +%s) - t0 ))"
[ "$fail" -eq 0 ] || exit 1
exit 0
