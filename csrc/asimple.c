/*
 * asimple.c -- short idle / gesture actions.
 *
 * Ports for doAct() handlers that don't need walking to a specific
 * house position and involve mostly head/body animation with sound.
 *
 * addr: a_wakfa(), a_hello(), a_yawas(),
 *       a_nodh(), a_petd(), a_calld()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern BOOL16   introSeq;
extern short    g_trel[];
extern BOOL16   alarm_p;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern long     g_sfret;
extern BOOL16   g_actif;
extern BOOL16   dg_petok;
extern short    g_wtx;
extern short    g_wty;
extern short    pst_arr[];
extern void     lcp_hwt();
extern void     gameTick();
extern void     hs_posXY();
extern short    lcp_st;
extern short    lcp_face;
extern short    rndRng();                  /* random.c */
extern void     doAct();                    /* actions.c */
extern short    rndRng();
extern short    lcp_wkD();
extern void     p_sftvc();
extern void     p_sfgrt();
extern void     p_sfspe();
extern void     p_sfhnd();
extern void     sf_so();

/* a_wakfa: Ctrl+A path.  Walks to the bedroom alarm,
   faces right, silences the alarm and clears the pressed flag.
   addr: a_wakfa() */

void
a_wakfa()
{
        short   result;

        hs_posXY(POS_MID_BEDROOM_WALK,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result == 0) {
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                alarm_p = NO;
        }
}

/* a_hello: face-forward wave with a random 20-40 head sequence.
   Interruptible via the deferred-event queue.
   addr: a_hello() */

void
a_hello()
{
        short   wave_count;
        short   pick;
        short   prev_pick;
        short   saved_frame;
        short   wait;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_hwt();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        wave_count = rndRng(20, 40);
        prev_pick  = 0;
        pick       = 0;
        while (wave_count != 0) {
                while (pick == prev_pick)
                        pick = rndRng(0, 2);
                prev_pick = pick;

                if (pick == 0) {
                        g_hsfra = 5;
                        p_sftvc();
                } else if (pick == 1) {
                        g_hsfra = 6;
                        if (rndRng(0, 1) == 0)
                                p_sfgrt();
                        else
                                p_sfspe();
                } else {
                        g_hsfra = 4;
                        p_sfhnd();
                }
                wait = rndRng(1, 2);
                gameTick(wait);
                g_sfret = (long) wait;
                wave_count = wave_count - 1;
        }

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}

/* a_yawas: 15-frame idle yawn.
   addr: a_yawas() */

void
a_yawas()
{
        short   i;

        pst_arr[0]  = STATE_YAWN_MOUTH_OPEN;
        pst_arr[1]  = STATE_YAWN_STRETCH_ARMS;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        for (i = 0; i < 15; i = i + 1) {
                lcp_st = pst_arr[i & 1];
                gameTick(1);
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}

/* a_nodh: 3-frame nod with SFX.
   addr: a_nodh() */

void
a_nodh()
{
        short   saved_frame;

        pst_arr[0]  = STATE_WALK_FRAME_3_STEP;
        pst_arr[1]  = STATE_WALK_FRAME_4;
        pst_arr[2]  = STATE_WALK_FRAME_5;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_hwt();

        saved_frame            = g_hsfra;
        g_hatas = HEAD_ANIM_DISABLED;
        g_hacur      = HEAD_ANIM_DISABLED;

        g_hsfra = pst_arr[0];
        gameTick(1);
        g_hsfra = pst_arr[1];
        gameTick(1);
        g_hsfra = pst_arr[2];
        gameTick(2);

        g_hatas = 8;
        g_hacur      = 8;
        g_hsfra      = saved_frame;
        gameTick(0);
}

/* a_petd: call the dog if not already pettable, then wait
   100..200 frames (10 during intro) or until a new event queues.
   addr: a_petd() */

extern void a_calld();

void
a_petd()
{
        short   ticks;

        g_actif = YES;
        if (dg_petok == NO)
                a_calld();
        g_actif = NO;

        ticks = rndRng(100, 200);
        if (introSeq != NO)
                ticks = 10;

        do {
                ticks = ticks - 1;
                if (ticks == 0)
                        break;
                gameTick(0);
        } while (g_trel[0] == ACTION_NONE);

        dg_petok = NO;
        lcp_st         = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}

/* a_calld: walk to POS_BTM_DOG_FOOD, crouch, set dg_petok.
   Real Ghidra behaviour -- see previous session for the derivation.
   addr: a_calld() */

void
a_calld()
{
        short   result;

        hs_posXY(POS_BTM_DOG_FOOD, &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        lcp_face   = FACING_RIGHT;
        g_hatas = 8;
        lcp_hwt();
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(5);
        dg_petok = YES;
}
