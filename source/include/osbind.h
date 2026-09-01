/*
 * osbind.h -- osbind bindings for the port.
 *
 * On the Atari ST, Alcyon C ships an osbind.h with TRAP #1 (GEMDOS),
 * TRAP #13 (BIOS), and TRAP #14 (XBIOS) macros.  This file mirrors the
 * subset the port actually uses so we don't need the DK header directly
 * (which would drag in typedefs that collide with our locally-defined
 * enums / structs).
 *
 * Host builds (-DHOST) get separate C functions in savehost.c that
 * emulate the same call surface through stdio / stdlib.
 */

#ifndef OSBIND_H
#define OSBIND_H

/* XBIOS trap numbers -- properties of the ABI, not of any shim. */
#define XBIOS_Setpalette        6
#define XBIOS_Setscreen         5
#define XBIOS_Logbase           3
#define XBIOS_Physbase          2
#define XBIOS_Random            17
#define XBIOS_Giaccess          28

#ifdef HOST

#include <stdlib.h>             /* NULL */

/* Host stubs implemented in savehost.c.  Same names as the target
   osbind macros so the port source is call-site-identical. */
extern short    Fopen();
extern short    Fcreate();
extern long     Fread();
extern long     Fwrite();
extern short    Fclose();
extern void *   Malloc();
extern long     Mfree();
extern void *   Fgetdta();
extern short    Fsfirst();
extern short    Fsnext();
extern short    Cconis();
extern long     Crawcin();
extern void *   Super();
extern short    Dsetpath();

/* AES event / alert stubs. */
#define evnt_timer(ms, msh)     ((void) 0)
#define form_alert(def, txt)    (1)

/* XBIOS stubs. */
#define Random()                ((long) rand())
#define Giaccess(data, reg)     (0L)
#define Setpalette(pal)         ((void) (pal))
#define Setscreen(log,phys,rez) ((void) (log))
#define Logbase()               ((void *) 0)
#define Physbase()              ((void *) 0)
#define Vsync()                 ((void) 0)
#define Midiws(n, b)            ((void) 0)
#define Dosound(p)              ((void) (p))
/* Host builds don't run the MIDI timer, so these are no-ops. */
extern long     bios();
extern long     xbios();

/* access(2) is in <unistd.h> on POSIX; declared here so save.c doesn't
   need to drag POSIX headers in. */
extern int      access();

#else   /* !HOST -- target (Alcyon) build */

/* Alcyon's `gemdos()` (from DK OSBIND.O) is the trap #1 wrapper.
   Everything below is a thin macro over it, matching the standard
   OSBIND.H shapes.  The trap #13 (`bios`) and trap #14 (`xbios`)
   wrappers are declared here so the port can call them directly
   for calls the Alcyon system osbind.h doesn't macro-wrap
   (specifically Setexc + Xbtimer used by mq_intim). */
extern long     gemdos();
extern long     bios();
extern long     xbios();

#ifdef FAITHFUL

/* GEMDOS: LCP_ORG's binding macros padded every call to the opcode
   plus THREE arguments (0L fills unused slots), and applied NO
   casts -- each argument is pushed at the width of the source
   expression.  Byte-verified against LCP_ORG.PRG (verify_bytes.py):
   Fopen/Fcreate/Fsfirst carry one trailing 0L, one-arg calls carry
   two, zero-arg calls three, and Fread/Fwrite are already full.
   Callers must therefore pass each argument at the ROM's width
   (e.g. a word rwmode vs. Fopen("hyber", 0L) in lc_load). */
#define Fopen(n, m)             gemdos(0x3D, n, m, 0L)
#define Fcreate(n, a)           gemdos(0x3C, n, a, 0L)
#define Fread(h, n, b)          gemdos(0x3F, h, n, b)
#define Fwrite(h, n, b)         gemdos(0x40, h, n, b)
#define Fclose(h)               gemdos(0x3E, h, 0L, 0L)
#define Malloc(sz)              gemdos(0x48, sz, 0L, 0L)
#define Mfree(p)                gemdos(0x49, p, 0L, 0L)
#define Fgetdta()               gemdos(0x2F, 0L, 0L, 0L)
#define Fsfirst(p, a)           gemdos(0x4E, p, a, 0L)
#define Fsnext()                gemdos(0x4F, 0L, 0L, 0L)
#define Cconin()                gemdos(0x01, 0L, 0L, 0L)
#define Cconws(s)               gemdos(0x09, s, 0L, 0L)
#define Cconis()                gemdos(0x0B, 0L, 0L, 0L)
#define Crawcin()               gemdos(0x07, 0L, 0L, 0L)
#define Pterm(rc)               gemdos(0x4C, rc, 0L, 0L)
#define Super(ssp)              gemdos(0x20, ssp, 0L, 0L)
#define Dsetpath(p)             gemdos(0x3B, p, 0L, 0L)
#define Giaccess(d, r)          xbios(0x1C, d, r, 0L)
#define Dosound(p)              xbios(0x20, p, 0L, 0L)

/* XBIOS, padded by the same ROM rule (opcode + three args). */
#define Physbase()              xbios(2, 0L, 0L, 0L)
#define Logbase()               xbios(3, 0L, 0L, 0L)
#define Setscreen(l, p, r)      xbios(5, l, p, r)
#define Setpalette(p)           xbios(6, p, 0L, 0L)
#define Midiws(n, b)            xbios(12, n, b, 0L)
#define Random()                xbios(17)       /* ROM: bare, no padding */
#define Vsync()                 xbios(37, 0L, 0L, 0L)

/* BIOS #5 -- Setexc(vector, handler): install/query an exception vector.
   Passing handler = -1 queries the current vector without installing. */
#define Setexc(v, h)            ((long) bios(5, (short)(v), (long)(h)))

/* XBIOS #31 -- Xbtimer(timer, ctrl_reg, data_reg, vector): install an
   MFP timer handler.  timer 0 = Timer-A, 1 = Timer-B, 2 = Timer-C,
   3 = Timer-D; ctrl/data are the initial MFP prescaler and data
   register values.  Used by mq_intim to install the Timer-A ISR. */
#define Xbtimer(t, c, d, v)     ((long) xbios(31, (short)(t), (short)(c), (short)(d), (long)(v)))

#else   /* !FAITHFUL: the STX revision's convention */

/* LCP_STX.PRG was built against the older Alcyon distribution's
   OSBIND.H (alcyon2, 1985-05-30), whose macros pass ONLY the real
   arguments -- no 0L padding, no casts (except the documented int
   casts on Cconis/Fsfirst/Fsnext).  Byte-observed in the STX text:
   Fclose pushes just opcode+handle before jsr gemdos (text 0x11a
   trap-#1 veneer).  Shapes below mirror that header's subset. */
#define Fopen(n, m)             gemdos(0x3D, n, m)
#define Fcreate(n, a)           gemdos(0x3C, n, a)
#define Fread(h, n, b)          gemdos(0x3F, h, n, b)
#define Fwrite(h, n, b)         gemdos(0x40, h, n, b)
#define Fclose(h)               gemdos(0x3E, h)
#define Malloc(sz)              gemdos(0x48, sz)
#define Mfree(p)                gemdos(0x49, p)
#define Fgetdta()               gemdos(0x2F)
#define Fsfirst(p, a)           (int) gemdos(0x4E, p, a)
#define Fsnext()                (int) gemdos(0x4F)
#define Cconin()                gemdos(0x01)
#define Cconws(s)               gemdos(0x09, s)
#define Cconis()                (int) gemdos(0x0B)
#define Crawcin()               gemdos(0x07)
#define Pterm(rc)               gemdos(0x4C, rc)
#define Super(ssp)              gemdos(0x20, ssp)
#define Dsetpath(p)             gemdos(0x3B, p)
#define Giaccess(d, r)          xbios(28, d, r)
#define Dosound(p)              xbios(32, p)
#define Physbase()              xbios(2)
#define Logbase()               xbios(3)
#define Setscreen(l, p, r)      xbios(5, l, p, r)
#define Setpalette(p)           xbios(6, p)
#define Midiws(n, b)            xbios(12, n, b)
#define Random()                xbios(17)
#define Vsync()                 xbios(37)
#define Setexc(v, h)            bios(5, v, h)
#define Xbtimer(t, c, d, v)     xbios(31, t, c, d, v)

#endif  /* FAITHFUL */

#endif  /* HOST */

#endif  /* OSBIND_H */
