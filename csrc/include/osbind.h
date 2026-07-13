/*
 * osbind.h -- host stub for Alcyon's <osbind.h>.
 *
 * On the Atari ST, Alcyon C ships an osbind.h with TRAP #13 (BIOS),
 * TRAP #14 (XBIOS), and TRAP #1 (GEMDOS) macros.  Host builds don't
 * have this, so we provide the same names as thin wrappers that stub
 * out the hardware side and forward to plausible host equivalents
 * (e.g. Random -> stdlib rand()).
 *
 * When building with Alcyon under Hatari, this file is shadowed by
 * the real osbind.h in the Alcyon include path -- do NOT keep any
 * ST-specific behaviour here, only host stand-ins.
 */

#ifndef OSBIND_H
#define OSBIND_H

#ifdef HOST

#include <stdlib.h>             /* rand(), NULL */
#include <stdio.h>              /* FILE, fopen, fread, fwrite, fclose */

/* XBIOS Random() (function 17): returns a 24-bit unsigned random.
   The stdlib rand() maxes out at RAND_MAX which varies by libc, so we
   only guarantee the 15 low bits are populated -- randomRange() only
   consults bits 0..14 anyway. */
#define Random()        ((long) rand())

/* GEMDOS trap #1 shim.  Called via the _gemdos(fn, ...) helper the
   original code uses.  On the host we route Fopen/Fread/Fwrite/Fclose/
   Fcreate through stdio; unhandled trap numbers become no-ops so a
   miswritten caller is a compile-time or runtime nuisance, not a crash.
   The int handle we return is a FILE* cast to short -- fine for the
   ports we currently need, which don't do handle arithmetic.

   ANSI prototype: K&R default argument promotions turn `short` into
   `int` on the caller side but leave `long` slots at their full 8-byte
   width on 64-bit hosts, so mixed short/long args across the ABI
   without a prototype misalign the register/stack layout.  Prototype
   at the call site forces the compiler to pass each arg at its
   declared width. */
extern long     host_gemdos_trap(short fn, long a, long b, long c);
#define _gemdos host_gemdos_trap

/* AES evnt_timer + form_alert stubs. */
#define evnt_timer(ms, msh)     ((void) 0)
#define form_alert(def, txt)    (1)

/* XBIOS Giaccess (function 28): read/write YM2149 PSG register.
   On the host the PSG doesn't exist -- return 0 so audio-reactive
   code (a_plawr's amp polling) sees silence. */
#define Giaccess(data, reg)     (0L)

/* Palette/screen XBIOS traps.  Host stubs are no-ops (palette /
   screen buffer manipulation lives in the graphics layer). */
#define Setpalette(pal)         ((void) (pal))
#define Setscreen(log,phys,rez) ((void) (log))
#define Logbase()               ((void *) 0)

/* Alcyon _xbios(fn, ...) helper used by the 1985 source.  On the host
   the trap number is used to route to the right macro above.  Only
   the calls that are actually issued are handled here; anything else
   returns 0.  This is a compile-time dispatch via ordinary if/else so
   the compiler folds it into the right XBIOS call at optimisation. */
#define XBIOS_Setpalette        6
#define XBIOS_Setscreen         5
#define XBIOS_Logbase           3
#define XBIOS_Random            17
#define XBIOS_Giaccess          28

extern long             host_xbios_trap(short fn, long a, long b, long c);
#define _xbios          host_xbios_trap

/* VDI (TRAP #2) wrappers.  Under Alcyon C's GEM binding these are C
   functions that stuff arguments into contrl/intin/ptsin parameter
   blocks and issue trap #2.  On the host they resolve to no-op K&R
   stubs in stubs.c -- the sprite pipeline builds its own image
   buffers, so the graphics wrappers only matter under Hatari. */

/* access(2) is in <unistd.h> on real POSIX but not always visible
   through <stdio.h>; provide a portable declaration so save.c doesn't
   need to include <unistd.h> and drag in POSIX-specific decls. */
extern int      access();

/* Everything else surfaces as a compile-time error if a non-random ST
   call sneaks in during a host build -- caught early rather than
   silently no-op'd. */

#endif  /* HOST */

/* XBIOS trap numbers -- true across host and target since they're
   properties of the ST XBIOS calling convention, not of our host shim.
   Declared outside the HOST block so target builds (and non-HOST
   subsystems like midi_seq.c) can reference them. */
#ifndef XBIOS_Setpalette
#define XBIOS_Setpalette        6
#define XBIOS_Setscreen         5
#define XBIOS_Logbase           3
#define XBIOS_Random            17
#define XBIOS_Giaccess          28
#endif

#endif  /* OSBIND_H */
