# source/tools/

**Build, verify, test** -- what `run_all.sh` drives:
`alcyon_build.sh`, `alcyon_link.sh` (with `cp_encrypt.py`, `bss_remap.py` and the
`stx_bss_layout.tsv` spec), `prg_diff.py`, `reloc_audit.py`,
`stx_check.sh` (`verify_bytes.py`, `ppbalance.py`), the Hatari tests
`test_keyboard.sh`, `test_actions.sh`, `test_saveload.sh`,
`test_longrun_stable.sh` (with `hatari_probe.sh`), and `run_hatari.sh` /
`frame_hash.sh`.  `check_truncation.py` (Makefile) guards the linker's
8-character symbol limit.

**Diagnosis:** `fn_diff.py` (one function, port vs reference),
`stx_txtdiff.py` (whole-text compare), `xbin_diff.py` (shared bytes with
another binary), `stx_extract.py` (files out of a Pasti .stx), `prg.py`
(shared PRG reader).

**Ghidra:** `sync_ghidra_names.sh`, `gen_ghidra_verify.py`, `ghidra/`,
`../../docs/ghidra_globals_map.md`; `renames.tsv` maps the 2026-10-05 renames.

**Music:** `sngdump.py`, `psgrender.py`, `render_psg_all.py`,
`midicheck.py` -- decode and render the .SNG/.ORG songs.

**Graphics:** `spritesheet.py` renders `DATA/SPRITES` and `DATA/OBJECTS`
with each graphic's name into `docs/images/sprites.png` and `objects.png`.

**Toolchain:** `build_toolchain.sh` rebuilds the Alcyon host tools.

**archive/** -- one-off tools from the function-matching campaign
(`stx_addrs`, `stx_locate`, `stx_neighbor`, `stx_whatis`, `stx_objmap`,
`stx_map`, `stx_strides`, `stx_unverified`, `find_syms`).  The binary is
byte-identical now, so nothing runs them; they still work from there.
