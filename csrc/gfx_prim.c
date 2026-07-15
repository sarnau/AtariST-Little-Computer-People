/*
 * gfx_prim.c -- low-level graphics primitives.
 *
 * These sit above the raw VDI/XBIOS traps but below the compositing
 * pipeline in render.c.  Each function is a real port of a Ghidra-
 * verified routine; the underlying VDI wrappers (vsl_color, v_pline,
 * vswr_mode, vsf_color, vsf_interior, vsf_style) are extern stubs in
 * stubs.c that map to the real trap #2 dispatch under Alcyon.
 *
 * addr: draw_line(), sc_sdtb(),
 *       sc_sdtf(), sc_firw(),
 *       blkcopy32()
 */

#include "types.h"
#include "enums.h"
#include "structs.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    vdihandle;
extern short    _vdi_color_table[];
extern void *   g_dscp;
extern void *   g_srlgb;
extern void *   save_logbase;
extern void *   g_srptr;
#include <osbind.h>

extern void     vsl_color();
extern void     v_pline();
extern void     vswr_mode();
extern void     vsf_interior();
extern void     vsf_style();
extern void     vsf_color();
extern void     sc_sdtb();
extern void     sc_sdtf();
extern void     vro_cpyfm();
extern short    screen_scale_factor;
extern MFDB     MFDB_A;
extern MFDB     MFDB_screen_ptr;
extern unsigned char SCREEN_BUFFER_B[];

/* draw_line: Bresenham-ish line via VDI v_pline in backbuffer, then
   restore frontbuffer draw target.  The 2-point polyline maps directly
   to a single-segment line under VDI.
   addr: draw_line() */

void
draw_line(x1, y1, x2, y2, color)
short   x1;
short   y1;
short   x2;
short   y2;
short   color;
{
        short   pts[4];

        sc_sdtb();
        vsl_color(vdihandle, _vdi_color_table[color]);
        pts[0] = x1;
        pts[1] = y1;
        pts[2] = x2;
        pts[3] = y2;
        v_pline(vdihandle, 2, pts);
        sc_sdtf();
}

/* sc_sdtb: XBIOS Logbase() saves the current
   log-base pointer; Setscreen redirects VDI output to g_srptr (the
   off-screen buffer).  Also resets the VDI fill mode to solid black so
   subsequent fill calls have a well-known state.
   addr: sc_sdtb() */

void
sc_sdtb()
{
        g_srlgb = (void *) _xbios(XBIOS_Logbase, 0L, 0L, 0L);
        _xbios(XBIOS_Setscreen, (long) g_srptr, -1L, -1L);
        vswr_mode(vdihandle, MD_REPLACE);
        vsf_interior(vdihandle, VSFPATT);
        vsf_style(vdihandle, FILL_SOLID);
        vsf_color(vdihandle, COLOR_black);
}

/* sc_sdtf: restore the log-base pointer stashed
   by the backbuffer switch.
   addr: sc_sdtf() */

void
sc_sdtf()
{
        _xbios(XBIOS_Setscreen, (long) g_srlgb, -1L, -1L);
}

/* sc_firw: paint one row (160 bytes = 80 words = 20
   quads-of-4) with the low-res-mode 3-bitplane 0xFFF pattern.  Row is
   offset by `row * 160` bytes from `scrptr`.  Each 4-word write plants
   1 blank bit-plane (0x0000) + 3 solid planes (0xFFFF), which combines
   into palette entry 0xF (white in the LCP palette).
   addr: sc_firw() */

void
sc_firw(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

        scrptr = (unsigned short *)
                 ((char *) scrptr + (long) row * 160);
        for (i = 0; i < 20; i = i + 1) {
                scrptr[0] = 0x0000;
                scrptr[1] = 0xffff;
                scrptr[2] = 0xffff;
                scrptr[3] = 0xffff;
                scrptr = scrptr + 4;
        }
}

/* sc_firs: same row-stride shape as _white, but plants
   two blank planes (0x0000) followed by two solid planes (0xFFFF)
   instead of one + three.  In the LCP 4-bitplane low-res palette this
   produces the light-cyan house-background stripe used for the top
   status strip between menu screens.
   addr: sc_firs() */

void
sc_firs(scrptr, row)
unsigned short *        scrptr;
short                   row;
{
        short   i;

        scrptr = (unsigned short *)
                 ((char *) scrptr + (long) row * 160);
        for (i = 0; i < 20; i = i + 1) {
                scrptr[0] = 0x0000;
                scrptr[1] = 0x0000;
                scrptr[2] = 0xffff;
                scrptr[3] = 0xffff;
                scrptr = scrptr + 4;
        }
}

/* sc_firb: paint one row (80 words = 160 bytes) with
   zeros.  All 4 bitplanes off -> palette index 0 -> black in the LCP
   palette.  Used by fill_top_rect_with_background as the separator
   line between the text pane and the game area.
   addr: sc_firb() */

void
sc_firb(scraddr, row)
unsigned short *        scraddr;
short                   row;
{
        short   column;

        scraddr = (unsigned short *)
                  ((char *) scraddr + (long) row * 160);
        for (column = 0; column < 80; column = column + 1) {
                *scraddr = 0;
                scraddr = scraddr + 1;
        }
}

/* init_vdi_and_screen: mini-game VDI setup.  Same shape as
   sc_sdtb -- stash the current log-base, point
   VDI output at the off-screen dest buffer, reset fill mode -- but
   uses a separate save_logbase slot (since the mini-game may nest its
   own set_draw_to_backbuffer calls without clobbering ours) and sets
   the default fill colour to palette entry 0xC (light green in the
   LCP mini-game palette) instead of black.
   addr: init_vdi_and_screen() */

void
init_vdi_and_screen()
{
        save_logbase = (void *) _xbios(XBIOS_Logbase, 0L, 0L, 0L);
        _xbios(XBIOS_Setscreen, (long) g_dscp, -1L, -1L);
        vswr_mode(vdihandle, MD_REPLACE);
        vsf_interior(vdihandle, VSFPATT);
        vsf_style(vdihandle, FILL_SOLID);
        vsf_color(vdihandle, _vdi_color_table[0xc]);
}

/* exit_vdi_and_screen: restore the pre-mini-game log-base.
   addr: exit_vdi_and_screen() */

void
exit_vdi_and_screen()
{
        _xbios(XBIOS_Setscreen, (long) save_logbase, -1L, -1L);
}

/* _draw_pixel: single-pixel plot via a degenerate VDI polyline where
   both endpoints are the same (x, y).  Structurally identical to
   draw_line -- backbuffer switch, colour set, 2-point polyline draw,
   frontbuffer restore -- but the collapsed endpoints let the VDI take
   the single-pixel fast path in the polyline handler.

   Used by rp_anim for the needle sweep pixels
   and by the mini-games' cursor + score indicators.

   addr: _draw_pixel() (in the graphics-primitives cluster; exact
   ROM offset not captured here) */

void
_draw_pixel(x, y, color)
short   x;
short   y;
short   color;
{
        short   pts[4];

        sc_sdtb();
        vsl_color(vdihandle, _vdi_color_table[color]);
        pts[0] = x;
        pts[1] = y;
        pts[2] = x;
        pts[3] = y;
        v_pline(vdihandle, 2, pts);
        sc_sdtf();
}

/* blkcopy32: unrolled 32-byte block copy.  Copies count*32 bytes from
   src to dst, 8 longwords at a time -- the 1985 code was aiming at a
   MOVEM.L unrolling for maximum ST bus throughput.  Modern compilers
   turn a plain memcpy() into equivalent SIMD, but preserving the shape
   keeps the port byte-for-byte comparable when we do build under Alcyon.
   addr: blkcopy32() */

void
blkcopy32(src, dst, count)
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
                sp = sp + 8;    /* 8 longs = 32 bytes */
                dp = dp + 8;
                remaining = remaining - 1;
        } while (remaining != -1);
}

/* sprite_init_MFDB (Ghidra 0x16612): initialise a VDI MFDB
   descriptor.  Sets bitmap address, pixel dimensions, word-width
   (w/16), standard-format flag = 0 (device native), and always
   4 bitplanes for ST low resolution.  The first parameter is
   originally an nplanes hint but is hardcoded to 4 inside the
   function and effectively ignored -- kept in the signature to
   match the 1985 shape.
   addr: sprite_init_MFDB() */

void
sprite_init_MFDB(unused, mfdb, addr, w, h)
long    unused;
MFDB *  mfdb;
void *  addr;
short   w;
short   h;
{
        mfdb->fd_addr    = addr;
        mfdb->fd_w       = w;
        mfdb->fd_h       = h;
        mfdb->fd_wdwidth = w / 16;
        mfdb->fd_stand   = 0;
        mfdb->fd_nplanes = 4;
}

/* copy_screen (Ghidra 0x164FA): raster-copy the current physbase
   screen into the memory buffer described by pdesMFDB.  Source is
   MFDB_A whose fd_addr is NULL -- the VDI convention that means
   "device screen", so vro_cpyfm reads from the shifter's currently
   displayed video RAM.  Mode ALL_WHITE (=0) is the S=1 constant
   raster op (fill destination with 1s, no source involved).  With
   fd_addr=NULL, GEM's vro_cpyfm implementation on the ST snapshots
   the visible screen into pdesMFDB's buffer regardless of the mode.
   addr: copy_screen() */

void
copy_screen(handle, pdesMFDB)
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

/* setup_screen_buffer (Ghidra 0x16576): allocate and initialise the
   double-buffered compositing screen.

     1. Zero MFDB_A.fd_addr so future vro_cpyfm calls that source
        from it read the visible device screen.
     2. Point g_srptr at SCREEN_BUFFER_B + 0x12F, then align UP to
        the next 512-byte boundary.  The 0x12F offset is a header
        the original code carved out before the aligned buffer; the
        512-byte alignment is stricter than the shifter DMA's 256-
        byte requirement and matches the disassembly exactly.
     3. Populate MFDB_screen_ptr describing that buffer as a
        scale*320 x scale*200 device-format bitmap (scale=1 =
        low-res, so 320x200).  0x1D00 is the "unused" first arg to
        sprite_init_MFDB (originally an nplanes hint).
     4. Snapshot the currently visible physbase into the buffer via
        copy_screen so the first compositing frame has something
        sensible to blend on top of.
   addr: setup_screen_buffer() */

void
setup_screen_buffer()
{
        long    buf;

        MFDB_A.fd_addr = (void *) 0;
        buf = (long) SCREEN_BUFFER_B + 0x12FL;
        buf = (buf + 0x200L) & ~0x1FFL;
        g_srptr = (void *) buf;
        sprite_init_MFDB(0x1D00L, &MFDB_screen_ptr, g_srptr,
                         (short) (screen_scale_factor * 0x140),
                         (short) (screen_scale_factor * 200));
        copy_screen(vdihandle, &MFDB_screen_ptr);
}
