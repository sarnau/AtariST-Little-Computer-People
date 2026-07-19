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

/* GEMDOS: each macro casts args to their declared trap-ABI types
   (WORD=short, VOIDP/LONG=long) so callers can pass anything
   convertible and the trap handler reads correct byte sizes. */
#define Fopen(n, m)             ((short) gemdos(0x3D, (long)(n),  (short)(m)))
#define Fcreate(n, a)           ((short) gemdos(0x3C, (long)(n),  (short)(a)))
#define Fread(h, n, b)          ((long)  gemdos(0x3F, (short)(h), (long)(n),  (long)(b)))
#define Fwrite(h, n, b)         ((long)  gemdos(0x40, (short)(h), (long)(n),  (long)(b)))
#define Fclose(h)               ((short) gemdos(0x3E, (short)(h)))
#define Malloc(sz)              ((void *) gemdos(0x48, (long)(sz)))
#define Mfree(p)                ((long)  gemdos(0x49, (long)(p)))
#define Fgetdta()               ((void *) gemdos(0x2F))
#define Fsfirst(p, a)           ((short) gemdos(0x4E, (long)(p),  (short)(a)))
#define Fsnext()                ((short) gemdos(0x4F))
#define Cconis()                ((short) gemdos(0x0B))
#define Crawcin()               ((long)  gemdos(0x07))
#define Super(ssp)              ((void *) gemdos(0x20, (long)(ssp)))
#define Dsetpath(p)             ((short) gemdos(0x3B, (long)(p)))

/* BIOS #5 -- Setexc(vector, handler): install/query an exception vector.
   Passing handler = -1 queries the current vector without installing. */
#define Setexc(v, h)            ((long) bios(5, (short)(v), (long)(h)))

/* XBIOS #31 -- Xbtimer(timer, ctrl_reg, data_reg, vector): install an
   MFP timer handler.  timer 0 = Timer-A, 1 = Timer-B, 2 = Timer-C,
   3 = Timer-D; ctrl/data are the initial MFP prescaler and data
   register values.  Used by mq_intim to install the Timer-A ISR. */
#define Xbtimer(t, c, d, v)     ((long) xbios(31, (short)(t), (short)(c), (short)(d), (long)(v)))

#endif  /* HOST */

#endif  /* OSBIND_H */
