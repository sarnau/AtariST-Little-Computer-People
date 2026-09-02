/*
 * init.c -- boot-time init functions from Ghidra main() at 0x15546.
 * addr: lcp_crnd @ 0x169D8, cl_drini @ 0x233B4, cs_mvIn.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "adoors.h"
#include "afood.h"
#include "agames.h"
#include "ahouse.h"
#include "aidle.h"
#include "aleisure.h"
#include "assets.h"
#include "calendar.h"
#include "delivery.h"
#include "dog.h"
#include "events.h"
#include "gfx_prim.h"
#include "globals.h"
#include "init.h"
#include "keyboard.h"
#include "midi_seq.h"
#include "movement.h"
#include "parser.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tables.h"
#include "walk.h"


/* lcp_crnd -> parts/lcp_crnd.c (STX: 0x69d8, in the 0x400c object after rnd). */
#ifdef FAITHFUL
#include "parts/lcp_crnd.c"
#endif

/* cl_drini -> parts/cl_drini.c (STX: 0x133b4). */
#ifdef FAITHFUL
#include "parts/cl_drini.c"
#endif


/* st_titl (ROM 0x7fae): in THIS binary the "title screen" is a stub
   that defaults the owner name to "PLAYER" and the clock to noon,
   0-0-0 -- there is no interactive name/date/time entry.  (The
   916-byte interactive version previously here came from the other
   Ghidra image; its TOS v_gtext crash makes sense in hindsight.)
   addr: st_titl() */

void
st_titl()
{
        short   i;

        lcp.owner_name[0] = 'P';
        lcp.owner_name[1] = 'L';
        lcp.owner_name[2] = 'A';
        lcp.owner_name[3] = 'Y';
        lcp.owner_name[4] = 'E';
        lcp.owner_name[5] = 'R';
        for (i = 6; i < 24; i = i + 1)
                lcp.owner_name[i] = 0;
        dt_mon   = 0;
        date_day = 0;
        dt_year  = 0;
        t_hour   = 12;
        t_min    = 0;
}

/* dbg_prA was a dead debug helper LCP_ORG shipped between st_titl
   and mq_intim.  LCP_STX does not have it. */


/* mq_intim: in THIS ROM an empty stub (0x804e) -- no Xbtimer call
   exists anywhere in the binary; its ~1.5 KB music engine (0x8cce)
   runs without a Timer-A ISR.  The port KEEPS the other-image
   Timer-A sequencer for now (same policy as the minigames: retained
   working features), because the port's mq_* engine needs the ISR --
   without it a_plawr's wait-for-mi_play spins forever.  INTENTIONAL
   non-fidelity until the ROM's polled engine is recovered.
   addr: mq_intim() */

void
mq_intim()
{
#ifdef FAITHFUL
        /* ROM 0x804e: empty. */
#else
#ifdef SKIP_MIDI
        /* Test builds: Timer-A jitter breaks frame-hash goldens. */
        (void) 0;
#else
        g_mtpre = 100;
        g_mtdiv = 4;
        mi_svtv = Setexc(0x4d, -1L);
        Xbtimer(0, 5, 0x28, (long) mq_tick);
#endif
#endif  /* FAITHFUL */
}

/* cntSong -> parts/cntSong.c (STX: 0x400c -- the FIRST function of the 0x400c object). */
#ifdef FAITHFUL
#include "parts/cntSong.c"
#endif

/* STX grouping: cl_redrH (0x137d4), cl_drwH and drwLine (0x138d4)
   live in the 0xdece object, so stx_u2.c includes parts/cl_redrH.c,
   parts/cl_drwH.c and parts/drwLine.c.  Their FAITHFUL twins are in
   render.c and gfx_prim.c. */


/* initBRev -> parts/initBRev.c (STX 0x6804), with the builder it
   calls in parts/rv_bld.c right behind it; stx_u1.c includes both. */


/* cs_mvIn (ROM 0x8106): boot-state initializer.  In this binary the
   "moves in" moment is just placing the resident at the front door
   (300,190) and parking the dog -- there is no animated cutscene.
   addr: cs_mvIn() */

void
cs_mvIn()
{
        lcp_x = 300;
        lcp_y = 190;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hacur = 8;
        g_hamod = HEAD_ANIM_DISABLED;
        dog_x = 273;
        dog_y = 190;
        g_dtx = 0;
        g_dty = 0;
        g_dyx = 0;
        g_dyy = 0;
        dg_stair = NO;
        dg_idlcd = 20;
        dg_ltgtI = g_dgitx;
        dg_init = 0;
        sp_spud(SPRITE_DOG_LAY_DOWN, -1, 1);
        introSeq = NO;
}
