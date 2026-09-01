#!/usr/bin/env bash
# build_toolchain.sh -- (re)build the native Alcyon toolchain from the
# compiler collection.  Created after ~/hatari-c was lost (2026-09-01):
# everything the build needs is regenerable from
# ~/Hatari_C/Compiler/Alcyon/alcyon (Thorsten Otto's cleaned-up Alcyon
# sources -- "no changes that would lead to different code generation")
# plus the Atari DK headers.  Produces:
#
#   $DEST/bin/          cp68 c068 c168 as68 ar68 link68 relmod optimize
#   $DEST/src/          patched working copy of the sources
#   $DEST/TOOLS/INCLUDE DK system headers (+_DTA/MFDB, ^Z stripped)
#
# Host patches applied to the working copy (validated 2026-09-01 by a
# clean FAITHFUL rebuild coming out BYTE-IDENTICAL to LCP_ORG.PRG):
#   - SSIZE 8 -> 32 in cpp/preproc.h + parser/icode.h + cgen/icode.h:
#     the port needs >8-char macro/identifier significance (the .s
#     post-processing in alcyon_build.sh still truncates linkage
#     names to 8, as before).
#   - parser/init.c: admit unsigned element types into the array-
#     initializer fast paths (the stock parser errors "initializer
#     alignment" on the 2nd element of any unsigned array).
#   - parser/main.c: sighandler_t cast -> void(*)(int) for macOS.
#   - link68/lnkmess.c: quoted ../include/stdarg.h -> <stdarg.h> so
#     clang's include_next reaches the system header.
#   - hostinc/: the missing "common" host-compat headers (cout.h,
#     ar68.h, sendc68.h, stdarg.h, varargs.h, compiler.h).
#
# Known quirk: this cp68 crashes on very long (~120+ char) file
# paths; the repo's build paths are short enough.
#
# Env: ALCYON_SRC (default: the compiler collection), DEST.

set -euo pipefail

ALCYON_SRC=${ALCYON_SRC:-$HOME/Hatari_C/Compiler/Alcyon/alcyon}
DK_INC=${DK_INC:-$HOME/Hatari_C/Compiler/ATARI_DK/DISK_1/COMPILER}
DEST=${DEST:-$HOME/Hatari_C/hatari-c}
CSRC=$(cd "$(dirname "$0")/.." && pwd)

mkdir -p "$DEST/bin"

# ---- 1. working copy of the sources -------------------------------
if [ ! -d "$DEST/src" ]; then
    cp -R "$ALCYON_SRC" "$DEST/src"
fi
cd "$DEST/src"

# ---- 2. host-compat include dir -----------------------------------
mkdir -p hostinc
cp -f include/cout.h include/ar68.h include/sendc68.h \
      include/stdarg.h include/varargs.h include/compiler.h hostinc/

# ---- 3. patches (idempotent) --------------------------------------
python3 - <<'PYEOF'
import re

def patch(path, old, new, required=True):
    s = open(path).read()
    if new in s:
        return
    if old not in s:
        if required:
            raise SystemExit(f'{path}: patch anchor missing')
        return
    open(path, 'w').write(s.replace(old, new))

patch('cpp/preproc.h', '#define\tSSIZE\t\t8', '#define\tSSIZE\t\t32')
for f in ('parser/icode.h', 'cgen/icode.h'):
    patch(f, '#define SSIZE       8               /* chars per symbol */',
             '#define SSIZE       32              /* chars per symbol */')

s = open('parser/main.c').read()
s = s.replace('(sighandler_t)', '(void (*)(int))')
open('parser/main.c', 'w').write(s)

s = open('link68/lnkmess.c').read()
s = s.replace('#include "../include/stdarg.h"', '#include <stdarg.h>')
open('link68/lnkmess.c', 'w').write(s)

patch('parser/init.c',
'''	if (type == (ARRAY | CHAR) || type == (ARRAY | INT) || type == (ARRAY | LONG))
	{
		nbout = str_init(datasize, type);
	} else if (atype == (ARRAY | CHAR) || atype == (ARRAY | INT) || atype == (ARRAY | LONG))''',
'''	if (type == (ARRAY | CHAR) || type == (ARRAY | INT) || type == (ARRAY | LONG) ||
	    type == (ARRAY | UCHAR) || type == (ARRAY | USHORT) ||
	    type == (ARRAY | UNSIGNED) || type == (ARRAY | ULONG))
	{
		nbout = str_init(datasize, type);
	} else if (atype == (ARRAY | CHAR) || atype == (ARRAY | INT) || atype == (ARRAY | LONG) ||
	    atype == (ARRAY | UCHAR) || atype == (ARRAY | USHORT) ||
	    atype == (ARRAY | UNSIGNED) || atype == (ARRAY | ULONG))''')
patch('parser/init.c',
'''	if ((datasize == CHARSIZE && BTYPE(type) == CHAR) ||	/* undimensioned array */
		(datasize == INTSIZE && (BTYPE(type) == INT || BTYPE(type) == SHORT)) ||
		(datasize == LONGSIZE && BTYPE(type) == LONG))''',
'''	if ((datasize == CHARSIZE && (BTYPE(type) == CHAR || BTYPE(type) == UCHAR)) ||	/* undimensioned array */
		(datasize == INTSIZE && (BTYPE(type) == INT || BTYPE(type) == SHORT ||
			BTYPE(type) == USHORT || BTYPE(type) == UNSIGNED)) ||
		(datasize == LONGSIZE && (BTYPE(type) == LONG || BTYPE(type) == ULONG)))''')
patch('parser/init.c',
'''	case UNSIGNED:
	case USHORT:
	case ARRAY | UNSIGNED:
		if''',
'''	case UNSIGNED:
	case USHORT:
	case ARRAY | UNSIGNED:
	case ARRAY | USHORT:
		if''')
patch('parser/init.c',
'''	case UCHAR:
		if (op == CINT || op == CLONG)''',
'''	case UCHAR:
	case ARRAY | UCHAR:
		if (op == CINT || op == CLONG)''')
patch('parser/init.c',
'''	case LONG:
	case ULONG:
	case ARRAY | LONG:
	case POINTER''',
'''	case LONG:
	case ULONG:
	case ARRAY | LONG:
	case ARRAY | ULONG:
	case POINTER''')
print('patches ok')
PYEOF

# ---- 4. build the host tools --------------------------------------
STD="-O -Wall -std=gnu89 -Wno-error=implicit-function-declaration"
make -C cpp    INC=../hostinc CFLAGS="$STD -D__intptr_t=intptr_t -Dlmalloc=malloc -Dlrealloc=realloc" >/dev/null
make -C parser INC=../hostinc >/dev/null
make -C cgen   INC=../hostinc CFLAGS="$STD -include stdarg.h -I../hostinc" >/dev/null
make -C as     INC=../hostinc CFLAGS="$STD -include stdarg.h -I../hostinc" >/dev/null
make -C util   CPPFLAGS="-I ../hostinc" >/dev/null
make -C link68 CPPFLAGS="-I ../hostinc -DGEMDOS" CFLAGS="$STD" >/dev/null
make -C optimize INC=../hostinc >/dev/null 2>&1 || true

cp -f cpp/cp68 parser/c068 cgen/c168 as/as68 util/ar68 \
      link68/link68 link68/relmod "$DEST/bin/"
cp -f optimize/optimize "$DEST/bin/" 2>/dev/null || true

# ---- 5. TOOLS/INCLUDE from the DK headers -------------------------
mkdir -p "$DEST/TOOLS/INCLUDE"
for f in "$DK_INC"/*.H; do
    cp -f "$f" "$DEST/TOOLS/INCLUDE/$(basename "$f" | tr 'A-Z' 'a-z')"
done
python3 - "$DEST/TOOLS/INCLUDE" <<'PYEOF'
import glob, os, sys
inc = sys.argv[1]
for f in glob.glob(os.path.join(inc, '*')):
    d = open(f, 'rb').read()
    if b'\x1a' in d:                        # CP/M EOF markers
        open(f, 'wb').write(d.replace(b'\x1a', b''))
PYEOF

cat > "$DEST/TOOLS/INCLUDE/ostruct.h" <<'HEOF'
/* ostruct.h -- GEMDOS structure layouts (reconstructed for the LCP
   port's TOOLS/INCLUDE; the standard 44-byte disk transfer area). */
#ifndef _OSTRUCT_H
#define _OSTRUCT_H

typedef struct {
        char            d_reserved[21];
        char            d_attrib;
        unsigned int    d_time;
        unsigned int    d_date;
        long            d_length;
        char            d_fname[14];
} _DTA;

#endif /* _OSTRUCT_H */
HEOF

grep -q "MFDB" "$DEST/TOOLS/INCLUDE/vdibind.h" || cat >> "$DEST/TOOLS/INCLUDE/vdibind.h" <<'HEOF'

/* Memory form definition block (VDI raster ABI); added for the LCP
   port -- the DK vdibind.h did not carry it. */
#ifndef _MFDB_DEFINED
#define _MFDB_DEFINED
typedef struct {
        char            *fd_addr;
        int             fd_w;
        int             fd_h;
        int             fd_wdwidth;
        int             fd_stand;
        int             fd_nplanes;
        int             fd_r1;
        int             fd_r2;
        int             fd_r3;
} MFDB;
#endif
HEOF

# ---- 6. GAME dir for Hatari runs ----------------------------------
mkdir -p "$DEST/GAME"
for f in "$CSRC/../DATA"/*; do
    b=$(basename "$f")
    case "$b" in LCP_ORG.PRG|LCP_STX.PRG|stx_extracted) ;; *)
        cp -f "$f" "$DEST/GAME/" ;;
    esac
done

echo "toolchain ready at $DEST/bin:"
/bin/ls "$DEST/bin"
