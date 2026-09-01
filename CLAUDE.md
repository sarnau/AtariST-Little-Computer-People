# Working notes for Claude / AI assistants on this repo

This is a **faithful C port** of the 1985 Atari ST game *Little Computer
People* (Activision).  The original LCP.PRG is disassembled in Ghidra
on the maintainer's machine and is the ground truth for every function,
initialization step, and control-flow decision.  The C source in
`source/` compiles under Alcyon C 4.14 (K&R) and runs on Hatari.

## Original toolchain (identified 2026-08-31)

The original LCP.PRG was built with **Alcyon C (DRI CP/M-68K C
compiler) as shipped in the official Atari ST Developer's Kit** — the
same Alcyon C 4.14 toolchain the port uses.  Fingerprinted from
`DATA/LCP_ORG.PRG` against the compiler collection in
`~/Hatari_C/Compiler/`:

- **Startup**: instruction-for-instruction the Atari DK
  `DISK_2/LINKER/GEMSTART.S` — including the "constant STACK value"
  fragment with **STACK=$8000 (32 KB stack)**, `MINSTACK=$1000`,
  `FUDGE=$200`, the `ext.w` command-length sequence, and the
  `tst.l (a7)+ / Pterm($4C)` exit.  The older `alcyon2` GEMSTART
  variant (`andi.l #$ff`, no stack models) does NOT match — it is
  specifically the DK revision.
- **Runtime library**: the DK's GEMLIB, wildcard `_main` (`xwmain.c`)
  linked in (strings `: unmatched quote`, `Cannot open/append/create`,
  `: No match`, `Stack Overflow`).  No LIBF traces — no floating point.
- **Control experiment**: the port's `source/build/alcyon/LCP.PRG`
  startup is byte-identical to the original modulo relocated absolute
  addresses.
- Ruled out: Pure C and Lattice 5.60 (postdate the game), Laser C /
  Megamax and Mark Williams C (different crt0 + runtime strings),
  early Lattice/Metacomco (no DRI-style wildcard `_main`).

## The rule: Ghidra-faithful means literal-faithful

Before writing or modifying any code path, verify it against the
original in Ghidra.  Match structure, order of operations, identifier
shape — **and every numeric literal, comparison operator, and sentinel
value**.

A shape-match audit ("both check `if (key)`, both set `tx_sctm=160`")
is NOT sufficient.  A single-token divergence can produce a silent
runaway bug:

**The getKey sentinel incident (2026-07-19)** — tick.c compared
getKey()'s result against the wrong no-key sentinel, so the "key
received" branch fired every tick, resetting `tx_sctm = 160`
continuously and locking the split-copy compositor.  Visible screen
corruption at ~11 500 VBLs, TOS bus error shortly after.  A
shape-audit had passed; literal-audit had not.  CAUTION (2026-09-01):
the sentinel POLARITY differs between the two game revisions -- in
DATA/LCP_ORG.PRG the tick-side test is `key != 0` (byte-verified),
while the other Ghidra image used `!= -1`.  Always verify against the
chosen reference binary, not against this anecdote.

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
`source/tools/test_longrun_stable.sh`.

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

- **Always use the Hatari MCP server for interactive testing** (AI
  assistants: registered as `hatari` in local MCP config; source in
  `~/Downloads/Hatari/`).  It drives a patched Hatari over a socket:
  keyboard/mouse injection, inline screenshots, resolution & CPU
  clock control, floppy/GEMDOS-drive mounting, and debugger
  memory/register reads.  Batch regression scripts in `source/tools/`
  keep their own Hatari invocations.

- LCP.PRG must be launched **directly** — from Hatari's `--auto`
  option or double-clicked from the GEM desktop.  Launching via
  COMMAND.PRG (Atari's shell) leaves the workstation in a state
  where subsequent `Setscreen` calls invalidate VDI line-attribute
  state, and `vsl_color` silently falls back to pen 15 (dark brown).
  This causes the water tank to render brown instead of blue — see
  the comment above `sc_sdtb` in `source/gfx_prim.c`.
- `SKIP_TITLE` no longer exists: the interactive title screen was an
  other-revision feature; st_titl is the ROM's defaults stub.  Test
  builds may still use `-DSKIP_MIDI=1` (skips the kept Timer-A
  install for frame-hash determinism).  `-DFAITHFUL=1` selects the
  byte-identical configuration (ROM minigame stubs, empty mq_intim).

## Key project layout

- `source/*.c` — the port itself.
- `source/include/*.h` — types, enums, struct layouts.
- `source/tools/` — build & test scripts (Alcyon build, Hatari-driven
  regression tests, symbol lookup helpers).
- `source/tests/` — host-side unit tests (compile under host cc, not
  Alcyon; not part of the shipped binary).

## Ghidra ↔ port cross-reference

`source/tools/ghidra_globals_map.md` documents every port global that
has a Ghidra counterpart with a different (usually longer) name.  To
push the map to the Ghidra project, run:

    source/tools/apply_ghidra_renames.sh

This regenerates a TSV via `gen_ghidra_rename_tsv.py`, then POSTs to
Ghidra's HTTP server at `localhost:8089/run_script` to invoke
`~/ghidra_scripts/RenameLcpGlobals.java`.  Prereqs: Ghidra open with
LCP.PRG loaded; `list_data_symbols.java` has been run at least once.

Struct name/field syncing uses the same HTTP mechanism via
`~/ghidra_scripts/RenameGhidraStructs.java` and a
`lcp_struct_rename_map.tsv`.

## Byte-fidelity campaign (2026-09-01)

`source/tools/verify_bytes.py` proves each port function byte-identical
to `DATA/LCP_ORG.PRG` (relocations and PC-relative displacements
wildcarded); `fn_diff.py NAME [orig_hex]` prints side-by-side
disassembly for recovering exact C.  Workflow: build, link, relink
without `-s` to `lcp_sym.68k`, run verify, fix, repeat.

Status: **303 matched / 57 divergent, 91.6% of the original text
proven byte-identical.**  Recovered so far: the ROM's own osbind.h
shapes (GEMDOS padded to opcode+3 args, NO argument casts; XBIOS
per-site), the od_* frame-id global tables (data 0x11758-0x1177e and
0x1200a-0x12026), WORD_TO_ACTION 14-byte rows, screen alignment
`(scrbufA+0xFF)&~0xFF` with alt screen at +0x8000, the shipped
(unsigned char)->ext.w Alcyon miscompile in sp_lcpf, several real
logic recoveries (chk_actT hunger gate, a_kitcc chew loop + g_actif,
dg_mvAn eating-sprite gate, ev_ansP od_med1, sf_sl is open+close
only -- SOUNDS.LCP is vestigial on ST), and lcp_save/lc_load/sgPlay
call shapes.

Second wave (after the maintainer confirmed LCP_ORG.PRG as the
reference): stair states hold the planted F3/F3S frames (walk core
lcp_pat/lcp_fst byte-MATCH); tv_boul/tv_patl draw a deliberate
pseudo-random second polyline point via frame-layout overlap (reverts
12e572f semantics); the game's OWN VDI binding layer exists at ROM
0xd664-0xd976 (vdiown.c + injected trap-#2 stub -- game code never
draws through VDIBIND); od_draw/sp_draw use the discrete-arg vro_cpy;
stpScrB/fillTopR share ONE buffer (dsb_stor was invented); rev_tab is
initialized DATA; and this binary has NO interactive title screen, NO
Timer-A install (st_titl defaults PLAYER/noon, mq_intim is empty), and
NO move-in cutscene (cs_mvIn is a state initializer).

Third wave: vdi_init matches ROM 0x7b72 (local work arrays, no
resolution check, inline mouse-off + screen clear; sc_ers deleted),
initVdi matches 0x78c8, vdi_cprt folded into vro_cpy.

**Policy decision (2026-09-01): the minigames are KEPT.**  The poker/
blackjack/word-puzzle/anagram suite (~26 KB), the Timer-A MIDI
sequencer (mq_*/psg_upE + mq_tick.s + the Xbtimer install in
mq_intim), and their support helpers (mg_wkev, lcp_lgt/lcp_rgt,
vst_h20, rst_vsth, moff, + linked vqt_att/vst_hei) come from the
OTHER, larger game revision and do not exist in LCP_ORG.PRG (whose
entire games area is ~1 KB at 0x72ac and whose music engine is ~1.5 KB
at 0x8cce, polled -- no ISR).  They are retained as intentional
non-fidelity and reported as KEPT by verify_bytes.py.

**GOAL (2026-09-01): a byte-identical LCP.PRG built from the
recreated C with the Alcyon toolchain.**  Enablers already in place:
`rom_map.py` reconstructs the ROM symbol map from matched relocation
operands (353 globals mapped -- see rom_data_map.txt) and proves the
ROM link order IS the port's alphabetical file order
(rom_link_order.txt).  Remaining phases:
 1. Code completeness (-DFAITHFUL): **DONE.**  ROM minigame banner
    stubs, dead stub 0x8030, empty mq_intim, the vestigial music
    engine (mq_dise 1460/1460 byte-identical; 10-byte PSG_ENVELOPE;
    Timer-A tail gated out and mq_tick.o dropped via FAITHFUL=1 in
    alcyon_link.sh), and the ROM's own workstation module
    (vdilib.c + vdilib_a.s in library position, shadowing VDIBIND's
    v_opnvwk/vro_cpyfm; a second runtime-patched parameter block
    vdipb2).  init.c's duplicate a_chfd deleted (aleisure.c a_chefd
    is the real one).  **The FAITHFUL text segment is SIZE-IDENTICAL
    to the ROM (70376 bytes) with every function at its exact ROM
    address**; the ~4.9 KB of differing text bytes are all relocated
    operands awaiting data/bss layout.  Use tools/prg_diff.py as the
    scoreboard (currently: data +1172, bss +3996, reloc 109 B off).
 2. Layout: redistribute globals from globals.c into their ROM
    defining objects in ROM data order per rom_data_map.txt; match
    string-literal ordering; drop port-only globals (_stksize etc.).
 3. prg_diff tool: whole-file compare (header/text/data/reloc).

Current: **308 matched / 50 kept, 95.1% coverage -- the
function-level campaign is COMPLETE for game code.**  `main` matches ROM 0x1ba (no Dsetpath, no
initBM call -- bm32or/bm32and stay zero at runtime, the ROM's own
dead code; ct_clrB lands at ROM-identical 0x42e via same-object bsr).
`gameTick` matches ROM 0xce28 (carrying mode returns via the restored
cy_yoff switch helper; getKey's no-key sentinel is 0 in this binary,
NOT -1 -- the 2026-07-19 incident's polarity belongs to the other
image).  The one listed "divergent", fl_ltpl, is extent pollution:
its body matches; symbol-less statics of the kept MIDI engine follow
it in link order.  Optional future work: recover the ROM's polled
music engine (0x8cce) and games shell (0x72ac) alongside the kept
versions.  Runtime smoke + long-run tests REQUIRED before trusting
the behavior changes (SOUNDS.LCP, tv polylines, stairs, single-buffer
compositing, vdi_init clear, gameTick Path-B return, key sentinel).

**OPEN QUESTION for the maintainer -- two Ghidra programs.**
`LCP.rep` contains TWO programs (`LCP.PRG.1` and `LCP.PRG.1.1`), and
port comments cite addresses from both (main@0x15546, gameTick@0x256a6,
st_titl@0x16de6, mq_tick 0x1219a vs 0x111b0 -- no single load base can
reconcile these against LCP_ORG.PRG offsets).  ~280 functions match
LCP_ORG.PRG byte-for-byte, but some port readings (12-byte parser rows,
crd_xa as initialized data at 0x2a4fe, the gameTick body) do not exist
in LCP_ORG.PRG at all and presumably come from the other image.
Before continuing on the remaining 87 divergent functions (games
ag_*/wp_*/pk_*, MIDI mq_*/psg_upE, VDI init cluster, main/st_titl,
gameTic, and the VDIBIND library revision), confirm WHICH binary is
the porting reference.  Everything committed so far is byte-verified
against DATA/LCP_ORG.PRG.

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
- **Stairs work in gameplay; the `test_stairs` harness was a false
  negative (removed 2026-07-21).**  The `TEST_STAIRS=1|2` hook in
  `cs_mvIn` warped the LCP straight to a stair-entry coordinate and
  called `lcp_wkD()`, bypassing the AI walk that normally delivers
  the resident to the stairs in a valid state.  From that synthetic
  warp `lcp_wkD` degenerated (samples never entered stair mode --
  `lcp_stR` stayed 0, no 9..24 stair states), which the harness
  misreported as "descends by falling through floors."  The
  maintainer confirmed real play walks stairs up/down correctly, and
  a byte-for-byte audit of `lcp_path`/`lcp_flwp` against Ghidra found
  them faithful -- so the FAIL was the harness, not the game.  The
  `TEST_STAIRS` hook (init.c) and both `test_stairs*.sh` scripts have
  been deleted.  (Task #33 "Fix player sliding through floor on
  stairs" remains correctly resolved.)
- **`cp_main` copy protection stubbed.**  Intentional non-fidelity
  documented in `source/stubs.c`; the ROM routine can't run under
  Hatari (flock + XOR decrypt + FDC signature check).
- **Music playback never verified in production build.**  Test
  builds use `-DSKIP_MIDI=1` which never actually exercises the
  audio ISR.  A fixed-input `.SNG` smoke test would close this gap.
