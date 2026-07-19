/*
 * gfx_prim.c -- low-level graphics primitives.
 *
 * These sit above the raw VDI/XBIOS traps but below the compositing
 * pipeline in render.c.  Each function is a real port of a Ghidra-
 * verified routine; the underlying VDI wrappers (vsl_color, v_pline,
 * vswr_mode, vsf_color, vsf_interior, vsf_style) are extern stubs in
 * stubs.c that map to the real trap #2 dispatch under Alcyon.
 *
 * addr: drwLine(), sc_sdtb(),
 *       sc_sdtf(), sc_firw(),
 *       blkcp32()
 */

#include "types.h"
#include "enums.h"
#include "structs.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern short    vdihnd;
extern short    vdi_colt[];
extern void *   g_dscp;
extern void *   g_srlgb;
extern void *   sv_lgb;
extern void *   g_srptr;
#include <osbind.h>

#include <vdibind.h>            /* v_pline, vsl_color, vsf_*, vst_*, vswr_mode, ... */
extern short    sv_vqta[];
extern void     sc_sdtb();
extern void     sc_sdtf();
extern short    scr_scal;
extern MFDB     MFDB_A;
extern MFDB     mf_scrp;
extern unsigned char scrbufB[];

/* drwLine: Bresenham-ish line via VDI v_pline in backbuffer, then
   restore frontbuffer draw target.  The 2-point polyline maps directly
   to a single-segment line under VDI.
   addr: drwLine() */

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

/* sc_sdtb: XBIOS Logbase() saves the current
   log-base pointer; Setscreen redirects VDI output to g_srptr (the
   off-screen buffer).  Also resets the VDI fill mode to solid black so
   subsequent fill calls have a well-known state.
   addr: sc_sdtb()

   Launcher note: if LCP is launched via COMMAND.PRG (the Atari shell),
   the shell's own workstation state doesn't survive our Setscreen
   here and vsl_color silently falls back to pen 15 (dark brown) --
   see the "water tank draws brown" report.  Launching LCP.PRG
   directly from the GEM desktop (or a launcher that gives us a
   fresh VDI workstation) avoids the issue. */

void
sc_sdtb()
{
        g_srlgb = (void *) Logbase();
        Setscreen(g_srptr, (void *)-1L, -1);
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, VSFPATT);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, COLOR_black);
}

/* sc_sdtf: restore the log-base pointer stashed
   by the backbuffer switch.
   addr: sc_sdtf() */

void
sc_sdtf()
{
        Setscreen(g_srlgb, (void *)-1L, -1);
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
   palette.  Used by fillTopR as the separator
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

/* initVdi: mini-game VDI setup.  Same shape as
   sc_sdtb -- stash the current log-base, point
   VDI output at the off-screen dest buffer, reset fill mode -- but
   uses a separate sv_lgb slot (since the mini-game may nest its
   own set_draw_to_backbuffer calls without clobbering ours) and sets
   the default fill colour to palette entry 0xC (light green in the
   LCP mini-game palette) instead of black.
   addr: initVdi() */

void
initVdi()
{
        sv_lgb = (void *) Logbase();
        Setscreen(g_dscp, (void *)-1L, -1);
        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, VSFPATT);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, vdi_colt[0xc]);
}

/* exitVdi: restore the pre-mini-game log-base.
   addr: exitVdi() */

void
exitVdi()
{
        Setscreen(sv_lgb, (void *)-1L, -1);
}

/* drwPixel: single-pixel plot via a degenerate VDI polyline where
   both endpoints are the same (x, y).  Structurally identical to
   drwLine -- backbuffer switch, colour set, 2-point polyline draw,
   frontbuffer restore -- but the collapsed endpoints let the VDI take
   the single-pixel fast path in the polyline handler.

   Used by rp_anim for the needle sweep pixels
   and by the mini-games' cursor + score indicators.

   addr: drwPixel() (in the graphics-primitives cluster; exact
   ROM offset not captured here) */

void
drwPixel(x, y, color)
short   x;
short   y;
short   color;
{
        short   pts[4];

        sc_sdtb();
        vsl_color(vdihnd, vdi_colt[color]);
        pts[0] = x;
        pts[1] = y;
        pts[2] = x;
        pts[3] = y;
        v_pline(vdihnd, 2, pts);
        sc_sdtf();
}

/* blkcp32: unrolled 32-byte block copy.  Copies count*32 bytes from
   src to dst, 8 longwords at a time -- the 1985 code was aiming at a
   MOVEM.L unrolling for maximum ST bus throughput.  Modern compilers
   turn a plain memcpy() into equivalent SIMD, but preserving the shape
   keeps the port byte-for-byte comparable when we do build under Alcyon.
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
                sp = sp + 8;    /* 8 longs = 32 bytes */
                dp = dp + 8;
                remaining = remaining - 1;
        } while (remaining != -1);
}

/* sprite_init_MFDB (Ghidra 0x16612) is already ported as sp_iniM in
   sprender.c -- we call it from stpScrB below rather than
   duplicating the body here. */
extern void     sp_iniM();

/* cpyScr (Ghidra 0x164FA): raster-copy the current physbase
   screen into the memory buffer described by pdesMFDB.  Source is
   MFDB_A whose fd_addr is NULL -- the VDI convention that means
   "device screen", so vro_cpyfm reads from the shifter's currently
   displayed video RAM.  Mode ALL_WHITE (=0) is the S=1 constant
   raster op (fill destination with 1s, no source involved).  With
   fd_addr=NULL, GEM's vro_cpyfm implementation on the ST snapshots
   the visible screen into pdesMFDB's buffer regardless of the mode.
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

/* stpScrB (Ghidra 0x16576): allocate and initialise the
   double-buffered compositing screen.

     1. Zero MFDB_A.fd_addr so future vro_cpyfm calls that source
        from it read the visible device screen.
     2. Point g_srptr at scrbufB + 0x12F, then align UP to
        the next 512-byte boundary.  The 0x12F offset is a header
        the original code carved out before the aligned buffer; the
        512-byte alignment is stricter than the shifter DMA's 256-
        byte requirement and matches the disassembly exactly.
     3. Populate mf_scrp describing that buffer as a
        scale*320 x scale*200 device-format bitmap (scale=1 =
        low-res, so 320x200).  0x1D00 is the "unused" first arg to
        sp_imfd (originally an nplanes hint).
     4. Snapshot the currently visible physbase into the buffer via
        cpyScr so the first compositing frame has something
        sensible to blend on top of.
   addr: stpScrB() */

extern short *  g_dsb;
extern void *   g_dscp;
extern short    dsb_stor[];

/* aes_init (Ghidra 0x167aa): AES + palette + physbase snapshot.
     appl_init();
     vdi_hnd = graf_handle(&gr_hwchar, &gr_hhchar, &gr_hwbox, &gr_hhbox);
     Setpalette(main_pal);
     sv_phb = Physbase();
   The four graf_handle out-parameters (character w/h and box w/h) are
   AES housekeeping globals; the port doesn't reference them elsewhere,
   so local storage is fine.  Notably this function does NOT call
   v_opnvwk -- that's vdi_init's job. */

#include <gembind.h>            /* appl_init, graf_handle, graf_mouse, form_alert */
extern short    main_pal[];
extern void *   sv_phb;
extern short    vdi_hnd;

void
aes_init()
{
        short   gr_hwchar, gr_hhchar, gr_hwbox, gr_hhbox;

        appl_init();
        vdi_hnd = graf_handle(&gr_hwchar, &gr_hhchar,
                                 &gr_hwbox,  &gr_hhbox);
        Setpalette(main_pal);
        sv_phb = (void *) Physbase();  /* XBIOS Physbase */
}

extern short    scr_scal;
extern short    workin[];        /* ROM global at 0x47ea8 (11 shorts) */
extern short    work_out[];      /* ROM global at 0x4d218 (57 shorts) */

#define REZ_ST_MEDIUM   1
#define REZ_ST_HIGH     2
#define M_OFF           256

/* vdi_erase_screen (Ghidra 0x166fe): turn off the mouse then fill the
   whole screen with COLOR_black via v_bar.  Rectangle extents depend
   on scr_scal (low/medium: 319x199, high: 639x399).  Trailing vsf_color
   restores the default fill colour to palette slot 1 (Ghidra labels
   COLOR_green_sea; port's color_enum names slot 1 as COLOR_olive --
   naming discrepancy only, byte value is 1). */

void
vdi_erase_screen()
{
        short   r[4];

        vswr_mode(vdihnd, MD_REPLACE);
        vsf_interior(vdihnd, VSFPATT);
        vsf_style(vdihnd, FILL_SOLID);
        vsf_color(vdihnd, COLOR_black);
        r[0] = 0;
        r[1] = 0;
        if (scr_scal == REZ_ST_HIGH) {
                r[2] = 639;
                r[3] = 399;
        } else {
                r[2] = 319;
                r[3] = 199;
        }
        graf_mouse(M_OFF, (void *) 0);
        v_bar(vdihnd, r);
        vsf_color(vdihnd, 1);
}

/* vdi_init (Ghidra 0x16680): open the virtual VDI workstation, verify
   ST-low or ST-medium resolution, and clear the screen.  Ghidra flow
   preserved byte-for-byte, including the form_alert reboot-loop when
   the resolution query returns >= 601 pixels wide (mono ST-high). */

void
vdi_init()
{
        short   i;

        vdihnd = vdi_hnd;
        for (i = 0; i < 10; i = i + 1)
                workin[i] = 1;
        workin[10] = 2;
        v_opnvwk(workin, &vdihnd, work_out);
        scr_scal = REZ_ST_MEDIUM;
        if (work_out[0] < 601) {
                vdi_erase_screen();
                return;
        }
        for (;;)
                form_alert(0,
                        "[1][Must be in|low resolution.][REBOOT]");
}

void
stpScrB()
{
        long    buf;

        MFDB_A.fd_addr = (void *) 0;
        buf = (long) scrbufB + 0x12FL;
        buf = (buf + 0x200L) & ~0x1FFL;
        g_srptr = (void *) buf;
        /* dest_screenbase_ptr: independent 32 KB compositing buffer
           written by fillTopR / prCh and
           read (blkcp32'd into the compositor screen) by
           screen_render_8hz.  Ghidra's `dest_scr_buffer + 0x7f`
           decompile is a constant-fold of the same align-up-to-512
           pattern used here for g_srptr (verified against raw disasm
           of fillTopR at 0x1686c:
              add.l #0x200, D0
              and.l #-0x200, D0     ; = & ~0x1FF).
           The old port hack `g_dsb = g_srptr - 254` aliased the two
           buffers, which made the top-strip content "visible" only
           because it overwrote g_srptr directly; that broke the
           letter-scroll compositing and every other feature that
           expects dest_screenbase to be an independent buffer. */
        buf = ((long) dsb_stor + 0x200L) & ~0x1FFL;
        g_dsb  = (short *) buf;
        g_dscp = (void  *) buf;
        sp_iniM(0x1D00L, &mf_scrp, g_srptr,
                         (short) (scr_scal * 0x140),
                         (short) (scr_scal * 200));
        cpyScr(vdihnd, &mf_scrp);
}

/* vst_h20: save current VDI text attributes into sv_vqta and set
   text height to 20 pixels (mini-game title / answer render).
   addr: vdi_save_and_set_text_height_20() */

void
vst_h20()
{
        short   ta, tb, tc, td;
        vqt_attributes(vdihnd, sv_vqta);
        vst_height(vdihnd, 20, &td, &tc, &tb, &ta);
}

/* rst_vsth: restore VDI text height from the saved attribute block
   (sv_vqta[7] is the cell-height slot).
   addr: reset_vst_height() */

void
rst_vsth()
{
        short   ta, tb, tc, td;
        vst_height(vdihnd, sv_vqta[7], &td, &tc, &tb, &ta);
}

/* vdi_cprt: VDI raster-copy wrapper.  Packs the (src rect, dst rect)
   pair into an 8-short pxy array and dispatches vro_cpyfm.  Used by
   the mini-game card renderer.
   addr: vdi_copy_rect() */

void
vdi_cprt(handle, mode, srcMFDB, dstMFDB, x1a, y1a, x2a, y2a,
                                              x1b, y1b, x2b, y2b)
short   handle;
short   mode;
MFDB *  srcMFDB;
MFDB *  dstMFDB;
short   x1a, y1a, x2a, y2a;
short   x1b, y1b, x2b, y2b;
{
        short   pts[8];
        pts[0] = x1a; pts[1] = y1a; pts[2] = x2a; pts[3] = y2a;
        pts[4] = x1b; pts[5] = y1b; pts[6] = x2b; pts[7] = y2b;
        vro_cpyfm(handle, mode, pts, srcMFDB, dstMFDB);
}

/* moff: idempotent AES mouse hide.  Called before mini-game
   rendering so the GEM cursor doesn't leak onto the card sprites.
   moff_f prevents the second call from re-issuing M_OFF (cheap
   guard; AES tolerates repeated M_OFF but the guard preserves the
   original Ghidra semantics).
   addr: mouse_off() */

extern MFDB     mf_scb_c;
extern BOOL16   moff_f;

void
moff()
{
        if (moff_f == NO) {
                graf_mouse(M_OFF, (void *) 0);
                moff_f = YES;
        }
}

/* drwBar: solid-fill rectangle at (x1,y1)-(x2,y2) in `color`.
   Same shape as plEr (initVdi/v_bar/exitVdi bracket) but takes an
   explicit fill colour rather than using the fixed palette-0xC
   preset.  Used by the title-screen name/date/time input area
   which erases with COLOR_dk_brown before printing each field.
   addr: draw_bar_color() */

void
drwBar(x1, y1, x2, y2, color)
short   x1;
short   y1;
short   x2;
short   y2;
short   color;
{
        short   r[4];
        r[0] = x1; r[1] = y1;
        r[2] = x2; r[3] = y2;
        initVdi();
        vsf_color(vdihnd, vdi_colt[color]);
        v_bar(vdihnd, r);
        vsf_color(vdihnd, vdi_colt[0xc]);
        exitVdi();
}
