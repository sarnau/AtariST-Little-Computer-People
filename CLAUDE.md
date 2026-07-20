# Working notes for Claude / AI assistants on this repo

This is a **faithful C port** of the 1985 Atari ST game *Little Computer
People* (Activision).  The original LCP.PRG is disassembled in Ghidra
on the maintainer's machine and is the ground truth for every function,
initialization step, and control-flow decision.  The C source in
`csrc/` compiles under Alcyon C 4.14 (K&R) and runs on Hatari.

## The rule: Ghidra-faithful means literal-faithful

Before writing or modifying any code path, verify it against the
original in Ghidra.  Match structure, order of operations, identifier
shape — **and every numeric literal, comparison operator, and sentinel
value**.

A shape-match audit ("both check `if (key)`, both set `tx_sctm=160`")
is NOT sufficient.  A single-token divergence can produce a silent
runaway bug:

**The getKey `!= 0` incident (2026-07-19)** — tick.c had
`if (key != 0)` after `key = getKey()`.  Ghidra had
`if (keycode != keycode_enum_none)` where `keycode_enum_none == -1`.
`-1 != 0` is always true, so the "a key was received" branch fired
every tick, resetting `tx_sctm = 160` continuously and locking the
split-copy compositor forever.  Visible screen corruption at ~11 500
VBLs, TOS bus error shortly after.  A shape-audit had passed;
literal-audit had not been performed.

**Every audit must diff:**
- Numeric literals (`0`, `-1`, `0xFF`, `160`, `27`, hex constants).
- Comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`) — check
  operand values, not just the operator.
- Sentinel return values.  When Ghidra tests `foo() != foo_sentinel`,
  the port MUST test against the same sentinel, not a substitute like
  `0` or `NULL`.
- Bitmasks and shift counts.
- Loop bounds (`< N` vs `<= N` vs `< N+1`).

**Testing must be long enough to catch runaways.**  A corruption
caused by a per-tick reset takes ~3 min of gameplay to become
visible; it will not surface in a 25-second smoke test.  Regression
checks that exercise long-running gameplay (≥ 15 000 VBLs under
`--auto`) and diff a mid-run frame against an early-run frame catch
this class of bug in seconds — see
`csrc/tools/test_longrun_stable.sh`.

## Don't invent

- If a global reads NULL at runtime, find where the **original** sets
  it (init sequence, IRQ handler, asset loader) and port that.
  Don't invent a wrapper that sets it from `main()`.
- Don't invent glue functions like `init_compositing_screens()` or
  side pointers like `alt_screen_ptr` just because a global was
  uninitialized.  The 1985 code populated it somewhere; find that
  call site.
- Don't add error handling, fallbacks, or validation for scenarios
  that can't happen.  Trust internal code and framework guarantees.
- If a Ghidra decompile shows `if (x < 0)`, the port has `if (x < 0)`.
  Not `if (x <= 0)`, not `if (x == -1)`.

## Diagnostics are the exception

Sprinkling `gemdos(9, "…")` markers, dump-to-low-RAM writes, or
Hatari conditional breakpoints to localize a crash is fine.  Once
the diagnosis is done, **remove the test code**.  Ghidra-faithful
source ships; ad-hoc debug scaffolding does not.

## Launching / running the port

- LCP.PRG must be launched **directly** — from Hatari's `--auto`
  option or double-clicked from the GEM desktop.  Launching via
  COMMAND.PRG (Atari's shell) leaves the workstation in a state
  where subsequent `Setscreen` calls invalidate VDI line-attribute
  state, and `vsl_color` silently falls back to pen 15 (dark brown).
  This causes the water tank to render brown instead of blue — see
  the comment above `sc_sdtb` in `csrc/gfx_prim.c`.
- Test builds use `-DSKIP_TITLE=1 -DSKIP_MIDI=1` to bypass the title
  screen (`st_titl` triggers a TOS `v_gtext` crash at `$fd330c`)
  and disable Timer-A determinism issues.  Production builds omit
  those flags.

## Key project layout

- `csrc/*.c` — the port itself.
- `csrc/include/*.h` — types, enums, struct layouts.
- `csrc/tools/` — build & test scripts (Alcyon build, Hatari-driven
  regression tests, symbol lookup helpers).
- `csrc/tests/` — host-side unit tests (compile under host cc, not
  Alcyon; not part of the shipped binary).

## Ghidra ↔ port cross-reference

`csrc/tools/ghidra_globals_map.md` documents every port global that
has a Ghidra counterpart with a different (usually longer) name.  To
push the map to the Ghidra project, run:

    csrc/tools/apply_ghidra_renames.sh

This regenerates a TSV via `gen_ghidra_rename_tsv.py`, then POSTs to
Ghidra's HTTP server at `localhost:8089/run_script` to invoke
`~/ghidra_scripts/RenameLcpGlobals.java`.  Prereqs: Ghidra open with
LCP.PRG loaded; `list_data_symbols.java` has been run at least once.

Struct name/field syncing uses the same HTTP mechanism via
`~/ghidra_scripts/RenameGhidraStructs.java` and a
`lcp_struct_rename_map.tsv`.

## Known open issues

- **Real-time crash inside TOS VDI at address error / bus error.**
  Reproduces intermittently under Hatari real-time (never under
  `--fast-forward`).  Symptom: user code jumps to a wild PC in low
  RAM, then TOS cleanup cascades into `$fc9304` writing to
  wrapped-negative SP.  Root cause unknown after multiple audits
  (sign-extension audit clean, Timer-A ISR clean vs ROM 0x1219a,
  VSync/Setscreen pairing matches ROM, Super() bracketing not a
  race).  Next diagnostic: Hatari memwatch on `$25722` to catch the
  writer that clobbers `lcp_std`'s abs.L operand.  User has
  confirmed audio is off when it fires -- not a MIDI/PSG ISR issue.
- **`test_longrun_stable.sh` currently fails env-side.**  Multiple
  commits (including known-clean baselines) all report identical
  PSNR 24.479517 with varying bus-error counts (6-15).  Suggests
  Hatari cache / TOS ROM / `~/hatari-c/GAME/` state issue rather
  than a code regression.  Worth 30 min to nail down before it
  masks a real regression.
  *(Update 2026-07-21: resolved.  Root cause was tv_boul / tv_patl
  calling v_pline with count=2 but only initialising 1 point of the
  buffer; the second polyline endpoint was read from stack garbage,
  which corrupted the compositor over minutes and produced the
  earlier bus-error cascade.  Ghidra now shows the real short[4]
  buffer layouts; both fixed in commit 12e572f.  Long-run test now
  passes for 36000 VBLs / 10 real minutes.)*
- **`test_stairs.sh` / `test_stairs_up.sh` harness now works;
  underlying stair descent doesn't use stair mode.**  Two harness
  bugs fixed 2026-07-21:
    1. Scripts didn't pass `--auto 'C:\LCP.PRG'` to Hatari, so
       the game never launched -- Hatari booted to GEM desktop and
       every "0 bus errors" verdict was spurious.
    2. `BASE` was hardcoded to `0x13bbc`; under `--auto`, TOS's
       Pexec loads LCP.PRG's TEXT segment at `0x12596` instead
       (verified via Hatari's `info basepage` at VBL 5000).
  `find_syms.py` was never broken -- earlier apparent 88-byte drift
  was cross-build comparison (offsets from a clean init.o vs runtime
  from a TEST_STAIRS-enabled init.o).  A new `#ifdef TEST_STAIRS=1|2`
  hook in `cs_mvIn` (init.c) directly warps + calls `lcp_wkD()`,
  bypassing the AI action ladder.  With hook + `--auto` + correct
  BASE, samples now capture real game state: LCP does reach the
  bottom floor from the attic warp position, but `lcp_stR` never
  transitions to 1 and stair-state range 9..24 is never entered --
  meaning the port descends by falling through floors rather than
  via a proper staircase walk.  Task #33 ("Fix player sliding
  through floor on stairs") was verified via manual play in a
  different scenario; the automated harness surfaces the regression
  the manual test missed.
- **`cp_main` copy protection stubbed.**  Intentional non-fidelity
  documented in `csrc/stubs.c`; the ROM routine can't run under
  Hatari (flock + XOR decrypt + FDC signature check).
- **Music playback never verified in production build.**  Test
  builds use `-DSKIP_MIDI=1` which never actually exercises the
  audio ISR.  A fixed-input `.SNG` smoke test would close this gap.
