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


/* st_titl -> parts/st_titl.c (STX: 0x6d7e, in the 0x400c object after pa_skic). */
#ifdef FAITHFUL
#include "parts/st_titl.c"
#endif

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


/* cs_mvIn -> parts/cs_mvIn.c (STX: 0xe500, right after showLcp
   in the 0xdece object -- stx_u2.c includes it there). */
#ifdef FAITHFUL
#include "parts/cs_mvIn.c"
#endif
