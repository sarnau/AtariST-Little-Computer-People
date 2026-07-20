#!/usr/bin/env python3
"""gen_ghidra_rename_tsv.py -- generate the rename TSV for RenameLcpGlobals.java

Reads csrc/tools/ghidra_globals_map.md and Ghidra's symbol dump (produced
by ~/ghidra_scripts/list_data_symbols.java), emits address<TAB>ghidra_name
<TAB>port_short_name to ~/ghidra_scripts/lcp_rename_map.tsv.

Handles:
  - identifiers with optional [] suffix inside backticks
  - optional parenthetical annotation in the Ghidra column
  - Ghidra symbols exported with a leading U+FEFF BOM
  - leading-underscore variants in the map (some Ghidra symbols
    appear both with and without a leading _)

Skips identity pairs (Ghidra name == port name).
Deduplicates.

Usage:
  python3 csrc/tools/gen_ghidra_rename_tsv.py
"""
import os, re, sys

REPO   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MAP_MD = os.path.join(REPO, "tools", "ghidra_globals_map.md")
SYMS   = "/tmp/ghidra_syms.txt"                        # from list_data_symbols.java
OUT    = os.path.expanduser("~/ghidra_scripts/lcp_rename_map.tsv")

ROW = re.compile(
    r'^\|\s*`([a-zA-Z_][a-zA-Z0-9_]*)(?:\[\])?`'      # ghidra name (backticks, optional [])
    r'\s*(?:\([^)]*\))?'                              # optional (annotation)
    r'\s*\|\s*'
    r'`([a-zA-Z_][a-zA-Z0-9_]*)(?:\[\])?`\s*\|',      # port name
    re.M)

with open(MAP_MD) as f:
    md = f.read()

pairs, seen = [], set()
for m in ROW.finditer(md):
    g, p = m.group(1), m.group(2)
    if g == p or (g, p) in seen:
        continue
    seen.add((g, p))
    pairs.append((g, p))

# Load symbol dump (Ghidra list_data_symbols.java output).
if not os.path.exists(SYMS):
    print(f"ERROR: {SYMS} missing (run ~/ghidra_scripts/list_data_symbols.java)",
          file=sys.stderr)
    sys.exit(1)
sym_addr = {}
with open(SYMS) as f:
    for line in f:
        parts = line.strip().split("\t")
        if len(parts) == 2:
            name = parts[1].lstrip("﻿")          # strip BOM
            sym_addr.setdefault(name, parts[0])

rows, missing = [], []
for g, p in pairs:
    key = g if g in sym_addr else g.lstrip("_")
    if key in sym_addr:
        rows.append((sym_addr[key], key, p))
    else:
        missing.append(g)

os.makedirs(os.path.dirname(OUT), exist_ok=True)
with open(OUT, "w") as f:
    for a, g, p in rows:
        f.write(f"{a}\t{g}\t{p}\n")

print(f"pairs_matched={len(rows)} missing={len(missing)} -> {OUT}")
for m in missing:
    print(f"  MISSING: {m}", file=sys.stderr)
