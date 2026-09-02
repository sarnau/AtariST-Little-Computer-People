/*
 * renderx.c -- palette, TV, screen-scroll, and prCh.
 * Split from render.c. All functions are Ghidra-verified.
 * addr: pa_cloc(), pa_skic(), lcp_upal(), td_line(),
 *       td_nois(), sc_sctd(), prCh()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

#include <vdibind.h>
#include <obdefs.h>
#include "gfx_prim.h"
#include "globals.h"
#include "random.h"
#include "renderx.h"

/* pa_cloc: pick random/configured CLOTHING_COLOR_ID (0..15),
   load prim/sec colours to palette slots 1,2. Overshoot falls back
   to lcp.clothing_color.
   addr: pa_cloc() */

void
pa_cloc()
{
        short   index;

        index = rndRng(0, 0x1f);
        if (index > 0xf)
                index = lcp.clothing_color;

        main_pal[1] = g_clcop[index];
        main_pal[2] = g_clcos[index];
        Setpalette(main_pal);
}

/* pa_skic: same as pa_cloc but 8-entry skin table.
   addr: pa_skic() */

void
pa_skic()
{
        short   index;

        index = rndRng(0, 0xf);
        if (index > 7)
                index = lcp.skin_color;

        main_pal[1] = skin_pal[index];
        main_pal[2] = skin_pal[index];
        Setpalette(main_pal);
}

/* lcp_upal: refresh sickness tint at palette slot 6.
   ST_PEACH (0x743) healthy, ST_SICK_GREEN (0x363) sick.
   Called from sim.c (recovery), health.c (onset), lc_load (HYBER restore).
   addr: lcp_upal() */

#ifdef FAITHFUL
void
lcp_upal()
{
        if (lcp.sickness_level == SICKNESS_HEALTHY)
                main_pal[6] = ST_PEACH;
        else
                main_pal[6] = ST_SICK_GREEN;
        Setpalette(main_pal);
}
#endif  /* FAITHFUL -- the STX revision keeps it in health.c */

/* td_line: draw 5-line rabbit-ear antenna on TV.
   Diagonal-up-right from (44..48, 51..49) to (44..48, 57..55).
   Colour: COLOR_white when off, random when on (static effect).
   addr: td_line() */

/* td_line -> parts/td_line.c (STX: 0xdece object, 0x13c8a,
   immediately after tt_off). */
#ifdef FAITHFUL
#include "parts/td_line.c"
#endif

/* td_nois -> parts/td_nois.c (STX: 0x13c74, immediately before td_line). */
#ifdef FAITHFUL
#include "parts/td_nois.c"
#endif

/* sc_sctd: 1-row block scroll on top text strip (letter typewriter wrap).
   Copies 13 rows of 40 words each downward, blanks top two rows to white.
   addr: sc_sctd() */

void
sc_sctd()
{
        short   row;
        char *  dest_ptr;
        char *  src_ptr;

        dest_ptr = (char *) g_dscp;
        src_ptr  = (char *) g_dscp;
        for (row = 0; row < 13; row = row + 1) {
                src_ptr = src_ptr + 320;
                blkcp32(src_ptr, dest_ptr, 10);
                dest_ptr = dest_ptr + 320;
        }
        sc_firw(g_dscp, 24);
        sc_firw(g_dscp, 25);
}

/* prCh: render one char via VDI.
   Sets logbase to backbuffer, MD_TRANS overlay via v_gtext, restores state.
   Setscreen (void*)-1 for phys/rez means "leave unchanged".
   addr: prCh() */

/* prCh -> parts/prCh.c (STX: 0xdece object, 0x16ede, immediately after strPr). */
#ifdef FAITHFUL
#include "parts/prCh.c"
#endif

/* rp_anim: sweep needle x=70..83 at y=42, 1px/frame, wrap at 0.
   If music playing and not browsing records, roll random VU LED (0..6)
   at y=47 and toggle lit/unlit (red if new mask overlaps g_ltpac, else black).
   g_ltlic/g_ltpac are 1985 shared-storage: also record-player state
   when no letter is being written.
   addr: rp_anim() */

void
rp_anim()
{
        unsigned short  rnd;
        short           col;

        if (g_ltlic >= 0)
                drwPixel(g_ltlic + 70, 42, COLOR_white);
        g_ltlic = g_ltlic - 2;
        if (g_ltlic < 0)
                g_ltlic = 13;
        drwPixel(g_ltlic + 70, 42, COLOR_black);

        if (mi_play == NO || g_rbact != NO)
                return;

        rnd = (unsigned short) Random();
        rnd = rnd & 7;
        if (rnd < 7) {
                g_ltpac = rec_ledt[rnd] ^
                                         g_ltpac;
                if ((rec_ledt[rnd] &
                     g_ltpac) == 0)
                        col = COLOR_black;
                else
                        col = COLOR_red;
                drwPixel(rnd * 2 + 66, 47, col);
        }
}

/* strPr: paint NUL-terminated string at (x,y) via prCh, 8px/char advance
   (8x8 system font used by status strip / game menu).
   addr: strPr() */

/* strPr -> parts/strPr.c (STX: 0xdece object, 0x16ea8, immediately before prCh). */
#ifdef FAITHFUL
#include "parts/strPr.c"
#endif
