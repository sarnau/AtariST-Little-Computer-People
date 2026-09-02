/*
 * gfx_prim.c -- low-level graphics primitives above VDI/XBIOS.
 * addr: drwLine(), sc_sdtb(), sc_sdtf(), sc_firw(), blkcp32()
 */

#include "types.h"
#include "enums.h"
#include "structs.h"
#include <osbind.h>

#include <vdibind.h>            /* v_pline, vsl_color, vsf_*, vst_*, vswr_mode, ... */
#include <obdefs.h>
#include "gfx_prim.h"
#include "globals.h"
#include "sprender.h"

/* drwLine: single-segment line via VDI v_pline (backbuffer, restore).
   addr: drwLine() */

#ifdef FAITHFUL
void
drwLine(x1, y1, x2, y2, color)
short   x1;
short   y1;
short   x2;
short   y2;
short   color;
{
        short   pts[4];

        sc_sdtb();
        vsl_color(vdihnd, vdi_colt[color]);
        pts[0] = x1;
        pts[1] = y1;
        pts[2] = x2;
        pts[3] = y2;
        v_pline(vdihnd, 2, pts);
        sc_sdtf();
}
#endif  /* FAITHFUL -- STX groups it with cl_drini in init.c */

/* sc_sdtb: stash Logbase, Setscreen->g_srptr, reset fill to solid black.
   addr: sc_sdtb()

   Launcher note: launching via COMMAND.PRG leaves VDI in a state where
   vsl_color silently falls back to pen 15 (dark brown) -- water tank
   renders brown.  Launch LCP.PRG directly (GEM desktop or --auto). */

/* sc_sdtb -> parts/sc_sdtb.c (STX: 0xdece object, in the 0xdece object). */
#ifdef FAITHFUL
#include "parts/sc_sdtb.c"
#endif

/* sc_sdtf: restore log-base after sc_sdtb.
   addr: sc_sdtf() */

/* sc_sdtf -> parts/sc_sdtf.c (STX: 0xdece object, in the 0xdece object). */
#ifdef FAITHFUL
#include "parts/sc_sdtf.c"
#endif

/* sc_firw: paint row (160 B) with 0x0FFF (palette entry 0xF, white).
   addr: sc_firw() */

void
sc_firw(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

#ifdef FAITHFUL
        scrptr = (unsigned short *)
                 ((char *) scrptr + (long) row * 160);
        for (i = 0; i < 20; i = i + 1) {
                scrptr[0] = 0x0000;
                scrptr[1] = 0xffff;
                scrptr[2] = 0xffff;
                scrptr[3] = 0xffff;
                scrptr = scrptr + 4;
        }
#else
        /* STX: a 16-bit row multiply and post-incremented stores. */
        (char *) scrptr += row * 160;
        for (i = 0; i < 20; i++) {
                *scrptr++ = 0x0000;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
#endif
}

/* sc_firs: paint row with 0x0033 (2 planes) -- light-cyan status stripe.
   addr: sc_firs() */

void
sc_firs(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

#ifdef FAITHFUL
        scrptr = (unsigned short *)
                 ((char *) scrptr + (long) row * 160);
        for (i = 0; i < 20; i = i + 1) {
                scrptr[0] = 0x0000;
                scrptr[1] = 0x0000;
                scrptr[2] = 0xffff;
                scrptr[3] = 0xffff;
                scrptr = scrptr + 4;
        }
#else
        (char *) scrptr += row * 160;
        for (i = 0; i < 20; i++) {
                *scrptr++ = 0x0000;
                *scrptr++ = 0x0000;
                *scrptr++ = 0xffff;
                *scrptr++ = 0xffff;
        }
#endif
}

/* sc_firb: paint row with 0 -> palette index 0 (black) separator.
   addr: sc_firb() */

void
sc_firb(scraddr, row)
unsigned short *        scraddr;
short                   row;
{
        short   column;

#ifdef FAITHFUL
        scraddr = (unsigned short *)
                  ((char *) scraddr + (long) row * 160);
        for (column = 0; column < 80; column = column + 1) {
                *scraddr = 0;
                scraddr = scraddr + 1;
        }
#else
        (char *) scraddr += row * 160;
        for (column = 0; column < 80; column++)
                *scraddr++ = 0;
#endif
}

/* initVdi: mini-game VDI setup.  Same shape as sc_sdtb but uses
   sv_lgb (nestable) and default fill = palette 0xC (light green).
   addr: initVdi() */

#ifdef FAITHFUL
void
initVdi()
{
        sv_lgb = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1L);
        vswr_mode(vdihnd, 1);
        vsf_interior(vdihnd, 1);        /* ROM 0x791c: 1, not PATTERN */
        vsf_style(vdihnd, 1);           /* ROM 0x792e: 1, not 8 */
        vsf_color(vdihnd, vdi_colt[0xc]);
}
#endif  /* FAITHFUL -- the STX build keeps it in games.c */

/* exitVdi: restore pre-mini-game log-base.
   addr: exitVdi() */

#ifdef FAITHFUL
void
exitVdi()
{
        Setscreen(sv_lgb, (void *)-1L, -1L);
}
#endif  /* FAITHFUL -- the STX build keeps it in games.c */

/* drwPixel -> parts/drwPixel.c (STX: 0x13930). */
#ifdef FAITHFUL
#include "parts/drwPixel.c"
#endif

/* blkcp32: unrolled 32-byte block copy (count * 32 bytes, 8 longs at a
   time -- MOVEM.L target).  Kept for byte-comparability.
   addr: blkcp32() */

void
blkcp32(src, dst, count)
void *  src;
void *  dst;
short   count;
{
        long *  sp;
        long *  dp;
        short   remaining;

        sp = (long *) src;
        dp = (long *) dst;
        remaining = count - 1;
        do {
                dp[0] = sp[0]; dp[1] = sp[1];
                dp[2] = sp[2]; dp[3] = sp[3];
                dp[4] = sp[4]; dp[5] = sp[5];
                dp[6] = sp[6]; dp[7] = sp[7];
                sp = sp + 8;
                dp = dp + 8;
                remaining = remaining - 1;
        } while (remaining != -1);
}

/* cpyScr (Ghidra 0x164FA): vro_cpyfm the physbase screen into pdesMFDB.
   Source MFDB_A.fd_addr=NULL is VDI "device screen" -- reads visible
   video RAM.  Mode ALL_WHITE (=0) irrelevant on ST with fd_addr=NULL.
   addr: cpyScr() */

void
cpyScr(handle, pdesMFDB)
short   handle;
MFDB *  pdesMFDB;
{
        short   points[8];

        points[0] = 0;
        points[1] = 0;
        points[2] = pdesMFDB->fd_w - 1;
        points[3] = pdesMFDB->fd_h - 1;
        points[4] = 0;
        points[5] = 0;
        points[6] = pdesMFDB->fd_w - 1;
        points[7] = pdesMFDB->fd_h - 1;
        vro_cpyfm(handle, ALL_WHITE, points, &MFDB_A, pdesMFDB);
}

/* stpScrB (Ghidra 0x16576): init double-buffered compositing screen.
   1. MFDB_A.fd_addr=NULL (future vro_cpyfm read device screen).
   2. g_srptr = scrbufB + 0x12F, aligned UP to 512.
   3. Populate mf_scrp as scale*320 x scale*200; 0x1D00 = unused nplanes.
   4. cpyScr physbase snapshot for first compositing frame.
   addr: stpScrB() */

/* aes_init (Ghidra 0x167aa): appl_init + graf_handle + Setpalette +
   physbase snapshot.  Does NOT call v_opnvwk (vdi_init's job). */

#include <gembind.h>            /* appl_init, graf_handle, graf_mouse, form_alert */

void
aes_init()
{
        short   gr_hwchar, gr_hhchar, gr_hwbox, gr_hhbox;

        appl_init();
        vdi_hnd = graf_handle(&gr_hwchar, &gr_hhchar,
                                 &gr_hwbox,  &gr_hhbox);
        Setpalette(main_pal);
        sv_phb = (void *) Physbase();
}


#define REZ_ST_MEDIUM   1
#define REZ_ST_HIGH     2
#define M_OFF           256

/* vdi_init (ROM 0x7b72): open a virtual workstation on the physical
   handle using LOCAL work arrays, scr_scal=1, reset drawing
   attributes, hide the mouse, and clear the whole screen.  The ROM
   has no resolution check and no reboot alert. */

void
vdi_init()
{
        short   work_in[11];
        short   wk_out[57];
        short   i;
        short   rect[4];

        vdihnd = vdi_hnd;
        for (i = 0; i < 10; i = i + 1)
                work_in[i] = 1;
        work_in[10] = 2;
        v_opnvwk(work_in, &vdihnd, wk_out);
        scr_scal = 1;
        vswr_mode(vdihnd, 1);
        vsf_interior(vdihnd, 1);
        vsf_style(vdihnd, 1);
        vsf_color(vdihnd, 0);
        rect[0] = 0;
        rect[1] = 0;
        rect[2] = 319;
        rect[3] = 199;
        graf_mouse(256, 0L);
        v_bar(vdihnd, rect);
}

void
stpScrB()
{
        long    buf;

        MFDB_A.fd_addr = (void *) 0;
        buf = (long) scrbufB + 0x12FL;
        buf = (buf + 0x200L) & ~0x1FFL;
        g_srptr = (void *) buf;
        /* ROM 0x7c84: the fillTopR base is simply g_srptr - 254;
           fillTopR adds the 254 back before drawing, so the top strip
           renders into the SAME buffer -- the ROM has no separate
           compositing buffer. */
        g_dsb = (short *) ((long) g_srptr + -254L);
        sp_iniM(0x1D00L, &mf_scrp, g_srptr,
                         (short) (scr_scal * 0x140),
                         (short) (scr_scal * 200));
        cpyScr(vdihnd, &mf_scrp);
}

/* vst_h20: save VDI attrs to sv_vqta; set text height 20 px.
   addr: vdi_save_and_set_text_height_20() */

#ifndef FAITHFUL
void
vst_h20()
{
        short   ta, tb, tc, td;
        vqt_attributes(vdihnd, sv_vqta);
        /* STX passes the four out-pointers in declaration order. */
        vst_height(vdihnd, 20, &ta, &tb, &tc, &td);
}
#endif

/* rst_vsth: restore VDI text height from sv_vqta[7] (cell height).
   addr: reset_vst_height() */

#ifndef FAITHFUL
void
rst_vsth()
{
        short   ta, tb, tc, td;
        vst_height(vdihnd, sv_vqta[7], &td, &tc, &tb, &ta);
}
#endif


/* moff: idempotent AES mouse hide (moff_f guards repeat M_OFF).
   addr: mouse_off() */


#ifndef FAITHFUL
void
moff()
{
        if (moff_f == NO) {
                graf_mouse(M_OFF, (void *) 0);
                moff_f = YES;
        }
}
#endif

