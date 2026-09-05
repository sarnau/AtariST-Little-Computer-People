/*
 * hostasm.c -- host stand-ins for the hand-written assembly.
 *
 * Five .s files carry code the ST needs and C cannot express:
 * psg_asm.s (the frameless PSG pokes), mq_tick.s (the Timer-A ISR and
 * the five bytes it keeps in TEXT), cp_asm.s (the copy protection),
 * blkcp_a.s (the unrolled block copy) and vdistx_a.s (the VDI trap
 * dispatcher).  `make test` links the host objects into real
 * executables, so it needs those symbols to exist.
 *
 * These are stubs, not emulations: they let the tests link and let the
 * pure-logic ones (parser, HYBER round-trip, LETTER.TXT, .SCN, sprite
 * and sound loaders) run.  Anything that actually depends on ST
 * hardware cannot be tested this way and is not meant to be.
 *
 * HOST ONLY.  alcyon_build.sh skips this file the same way it skips
 * savehost.c, so it can never reach the shipped binary.
 */

#ifdef HOST

#include "types.h"

/* --- psg_asm.s: direct YM2149 register writes --------------------- */
void psg_wr(reg, val)   short reg; short val;   { (void) reg; (void) val; }
void psg_mix(mask)      short mask;             { (void) mask; }
void mowrit(val)        short val;              { (void) val; }

/* --- mq_tick.s: the Timer-A ISR and the five bytes behind it ------ */
void  mq_tick()  { }
BOOL16 mi_dwrm;
BOOL16 mi_rlock;
short  g_mtpre;
char   psg_ntAc;
char   g_msmsa;

/* --- cp_asm.s: the copy protection -------------------------------- */
/* Non-zero, as a passing check returns -- the host has no FDC. */
long cp_main()   { return 0xf000000aL; }

/* --- blkcp_a.s: the unrolled longword block copy ------------------ */
void
blkcp32(src, dst, longs)
char *  src;
char *  dst;
short   longs;
{
        short   i;
        for (i = 0; i < longs * 8; i++)
                dst[i] = src[i];
}

/* --- vdistx_a.s: the VDI trap dispatcher and its two mode words --- */
short   wr_src;
short   wr_dst;
void    gsx1()   { }

/* --- XBIOS / AES entries with no host meaning --------------------- */
/* Setexc and Xbtimer install the Timer-A ISR; appl_init, graf_handle
   and graf_mouse are the four AES calls aes_init and the mouse
   wrappers make.  savehost.c covers the GEMDOS file layer; these had
   no host definition anywhere. */
long  Setexc(vec, addr)  short vec; long addr; { (void) vec; (void) addr; return 0L; }
void  Xbtimer(t, ctl, dat, vec) short t; short ctl; short dat; long vec;
                                 { (void) t; (void) ctl; (void) dat; (void) vec; }
short appl_init()   { return 0; }
short graf_handle(w, h, bw, bh) short *w; short *h; short *bw; short *bh;
{
        *w = 8; *h = 8; *bw = 8; *bh = 8;
        return 1;               /* a plausible VDI workstation handle */
}
short graf_mouse(mode, form) short mode; void *form;
                             { (void) mode; (void) form; return 1; }

/* --- osbind.h's raw trap entries ---------------------------------- */
long gemdos()    { return 0L; }
long xbios()     { return 0L; }

#endif /* HOST */
