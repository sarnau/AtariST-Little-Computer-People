/*
 * aleisure.c -- music, fireplace, couch, exercise, and lightweight
 * house-upkeep handlers.
 * addr: a_lists(), a_playp(), a_plawr(), a_lighf(), a_socwd(),
 *       a_sitae(), a_chefd(), a_cleau(), a_tidyh(), a_opcbc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "actions.h"
#include "adoors.h"
#include "ahouse.h"
#include "aleisure.h"
#include "asimple.h"
#include "delivery.h"
#include "events.h"
#include "globals.h"
#include "midi_seq.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"


/* a_lists: pick a random .sng file and start it playing.
   Uses lcp_food as a modulo index (1985 code reused the field).
   addr: a_lists() */

void
a_lists()
{
        short   result;
        short   index;
        _DTA *   dta_ptr;
        char *  filename;
        short   i;

        if (lcp_recP != NO)
                return;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        gameTick(2);
        li_loor();
        lcp_recP = YES;

        index = rndRng(0, lcp_food - 1) + 1;
        Fsfirst("*.sng", 0L);
        while ((index = index - 1) != 0)
                Fsnext();
        dta_ptr = (_DTA *) Fgetdta();
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
        sgPlay(filename);
}

/* a_playp: stop a currently-playing record so the resident can start
   writing/typing.  Walks to dance floor, drains MIDI buffer, frees it.
   addr: a_playp() */

void
a_playp()
{
        short   result;

        if (lcp_recP == NO)
                return;

        hs_posXY(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        gameTick(2);

        if (mi_play != NO) {
                mq_inis(mi_sbuf, g_momap);
                while (mi_play != NO)
                        ;
        }
        li_loor();
        lcp_recP = NO;
        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }
}

/* a_plawr: browse vinyl shelf, play a random .org file.  Animation is
   amplitude-reactive: poll PSG channel volumes via Giaccess and pick
   a browsing pose when any channel got louder.  Host PSG stub returns
   0 forever, so it holds the reach-right pose.
   addr: a_plawr() */

void
a_plawr()
{
        short           result;
        _DTA *           dta_ptr;
        char *          filename;
        long            xres;
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;

        pst_arr[0] = STATE_VINYL_REACH_R;
        pst_arr[1] = STATE_VINYL_IDLE;
        pst_arr[2] = STATE_VINYL_REACH_L;
        pst_arr[3] = STATE_VINYL_PULL_OUT;

        prev_a = prev_b = prev_c = 0;
        g_actif = YES;
        if (lcp_recP != NO)
                a_playp();
        g_actif = NO;

        hs_posXY(POS_TOP_RECORD_SHELF,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        g_rbact = YES;
        g_hamod = HEAD_ANIM_DISABLED;
        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        gameTick(4);

        lcp_st = STATE_VINYL_REACH_R;
        g_selaf[SPRITE_VINYL_RECORD] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_VINYL_RECORD);
        g_sepex[g_seslm[SPRITE_VINYL_RECORD]] = 146;
        g_sepey[g_seslm[SPRITE_VINYL_RECORD]] =  54;
        gameTick(1);

        i = rndRng(1, org_cnt);
        Fsfirst("*.org", 0L);
        while ((i = i - 1) != 0)
                Fsnext();
        dta_ptr = (_DTA *) Fgetdta();
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
        sgPlay(filename);

        g_hamod = HEAD_ANIM_WALKING;
        while (mi_play == NO)
                ;

        while (mi_play != NO) {
                xres = Giaccess(0, 8);  psg_a = (unsigned char) xres & 0x1f;
                xres = Giaccess(0, 9);  psg_b = (unsigned char) xres & 0x1f;
                xres = Giaccess(0, 10); psg_c = (unsigned char) xres & 0x1f;

                lcp_st = pst_arr[0];
                if (prev_a < psg_a || prev_b < psg_b || prev_c < psg_c) {
                        i = rndRng(1, 3);
                        while (pst_arr[i] == lcp_st)
                                i = rndRng(1, 3);
                        lcp_st = pst_arr[i];
                        if (pst_arr[3] == lcp_st) {
                                gameTick(0);
                                result = rndRng(1, 2);
                                lcp_st = pst_arr[result];
                        }
                }
                gameTick(0);
                prev_c = psg_c; prev_b = psg_b; prev_a = psg_a;
        }

        g_hamod = HEAD_ANIM_DISABLED;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_st = pst_arr[0];
        lcp_hwt();
        gameTick(8);

        lcp_st = STATE_STAND_FACING_SCREEN;
        g_selaf[SPRITE_VINYL_RECORD] = SPRITE_HIDDEN;
        sp_upds();
        gameTick(0);

        if (mi_sbuf != (char *) 0) {
                Mfree(mi_sbuf);
                mi_sbuf = (char *) 0;
        }
        g_rbact = NO;
}

/* a_lighf: firewood run from front-door pickup to the
   fireplace, then stoke animation with a random-facing shrug pattern
   and 2500..5000 tick fire-active countdown.
   addr: a_lighf() */

void
a_lighf()
{
        short   result;
        short   i;

        if (fire_act != NO)
                return;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opcfd(0);
        g_actif = YES;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();

        /* Sit-dog sprite waits at the porch. */
        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(40);
        showLcp();

        sp_ssco(SPRITE_FIREWOOD);
        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result)
                a_opcfd(1);

        hs_posXY(POS_BTM_FIREPLACE_LOGS,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FIREWOOD] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_hwt();

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_BEND_DOWN;      gameTick(1);
        lcp_st = STATE_REACH_FORWARD;  gameTick(1);
        lcp_st = STATE_STOKE_FIREPLACE;gameTick(1);

        /* Random-direction shrug for 10 ticks (feeding kindling). */
        for (i = 0; i < 10; i = i + 1) {
                lcp_face = rndRng(0, 1);
                gameTick(0);
        }

        fire_act        = YES;
        fire_dur = rndRng(2500, 5000);

        lcp_face = FACING_RIGHT;
        lcp_st = STATE_STAND_FACING_SCREEN;
        gameTick(0);
        g_actif = NO;
}

/* a_socwd: call the dog over, sit on the couch,
   pet the dog for 30..50 ticks then crouch back off the couch.
   addr: a_socwd() */

void
a_socwd()
{
        short   ticks;

        g_actif = YES;
        a_calld();
        g_actif = NO;
        if (g_trel[0] != ACTION_NONE) {
                lcp_st = STATE_STAND_SIDE_VIEW;
                gameTick(0);
                return;
        }

        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        lcp_y = lcp_y + 9;
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        lcp_y = lcp_y - 3;
        g_selaf[SPRITE_READING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_READING_1);
        g_sepex[g_seslm[SPRITE_READING_1]] = 221;
        g_sepey[g_seslm[SPRITE_READING_1]] = 172;

        ticks = rndRng(30, 50);
        lcp_st = STATE_SIT_COUCH_PETTING_DOG;
        while (ticks != 0 && g_trel[0] == ACTION_NONE) {
                gameTick(3);
                ticks = ticks - 1;
        }

        lcp_y = lcp_y + 3;
        lcp_st = STATE_SIT_COUCH_UPRIGHT;
        g_selaf[SPRITE_READING_1] = SPRITE_HIDDEN;
        sp_upds();
        g_hatas = 8;
        lcp_hwt();
        gameTick(3);

        lcp_y = lcp_y - 9;
        lcp_st = STATE_CROUCH_DOWN;
        gameTick(8);
        while (g_ptdoa != NO)
                gameTick(0);

        dg_petok = NO;
        lcp_st = STATE_STAND_SIDE_VIEW;
        lcp_face = FACING_RIGHT;
        gameTick(1);
}

/* a_sitae: stretch arms in 4-frame cycle for a random
   number of iterations.
   addr: a_sitae() */

void
a_sitae()
{
        short           result;
        unsigned short  duration;
        unsigned short  i;

        pst_arr[0] = STATE_EX_ARMS_CTR;
        pst_arr[1] = STATE_EX_ARMS_UP;
        pst_arr[2] = STATE_EX_ARMS_CTR;
        pst_arr[3] = STATE_EX_ARMS_WIDE;

        hs_posXY(POS_MID_COUCH,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 5;
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();

        duration = (unsigned short) Random();
        i = 0;
        while (i < ((duration & 0x7f) | 8) &&
               g_trel[0] == ACTION_NONE) {
                lcp_st = pst_arr[i & 3];
                if (lcp_st == STATE_EX_ARMS_CTR)
                        gameTick(0);
                else
                        gameTick(3);
                i = i + 1;
        }
        lcp_st = STATE_STAND_SIDE_VIEW;
        gameTick(0);
}

/* a_chefd: walk to the door, open it, look outside for
   `value` ticks, then optionally close.  value is passed by the doAct
   dispatcher as 40 (see actions.c switch).
   addr: a_chefd() */

void
a_chefd(value)
short   value;
{
        short   result;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        if (lcp_frdO == NO)
                a_opcfd(0);
        g_actif = YES;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(value);
        showLcp();

        hs_posXY(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result) {
                g_actif = YES;
                hs_posXY(POS_BTM_FRONT_DOOR,
                                      &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(1);
        }
        g_actif = NO;
}

/* a_tidyh: walk to filing cabinet, possibly close it.
   addr: a_tidyh() */

void
a_tidyh()
{
        short   result;

        hs_posXY(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_watat();

        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result ||
            introSeq != NO)
                a_opcfc();
}

/* a_cleau: sweep all open doors/cabinets and close them.
   Order: upstairs first so downstairs animations don't collide with
   the toilet-door sprite pipeline.
   addr: a_cleau() */

void
a_cleau()
{
        short   result;

        if (lcp_flcO != NO) {
                hs_posXY(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
                if ((result = lcp_wkD()) != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfc();
        }
        if (studyDrO != NO) {
                hs_posXY(POS_TOP_STUDY_DOOR,
                                      &g_wtx, &g_wty);
                if ((result = lcp_wkD()) != 0)
                        return;
                lcp_face = FACING_RIGHT;
                lcp_st            = STATE_STAND_FACING_SCREEN;
                /* Ghidra shows a stray D0 = g_hatas - 12 here (Ghidra
                   0x1e9a0), but lcp_hwt is void and never reads D0 --
                   dead compiler artifact.  Port uses plain no-arg call. */
                lcp_hwt();
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(OBJ_DOOR_STUDY_OPEN_1, 178, 23);
                gameTick(2);
                od_draw(OBJ_DOOR_STUDY_CLOSED,  178, 23);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                gameTick(2);
                studyDrO = NO;
                lcp_face = FACING_RIGHT;
                lcp_st = STATE_STAND_FACING_SCREEN;
                gameTick(0);
        }
        if (lcp_toiO != NO) {
                hs_posXY(POS_MID_TOILET_DOOR,
                                      &g_wtx, &g_wty);
                if ((result = lcp_wkD()) != 0)
                        return;
                a_clotd();
        }
        if (lcp_clsO != NO) {
                hs_posXY(POS_MID_BEDROOM_CLOSET,
                                      &g_wtx, &g_wty);
                if ((result = lcp_wkD()) != 0)
                        return;
                a_clocd();
        }
        if (lcp_drsO != NO) {
                hs_posXY(POS_MID_DRESSER,
                                      &g_wtx, &g_wty);
                if ((result = lcp_wkD()) != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecd(1);
        }
        if (lcp_cabO != NO) {
                hs_posXY(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                if ((result = lcp_wkD()) != 0)
                        return;
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecc(1);
        }
        if (lcp_frdO != NO) {
                wkFrDr();
                lcp_face   = FACING_RIGHT;
                lcp_st              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(1);
        }
}

/* a_opcbc: 3-sprite dress-change sequence.
   Door swings open, 3-frame in-closet animation with palette swap
   (clothing/skin), door swings back.
   value=0 -> pa_cloc, value=1 -> pa_skic.
   addr: a_opcbc() */

void
a_opcbc(value)
short   value;
{
        short   result;
        short   saved_x;
        short   counter;

        hs_posXY(POS_MID_DRESSER,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        a_opecd(0);
        counter = rndRng(0, 100);
        if (lcp.initiative_threshold < counter)
                a_opecd(1);

        hs_posXY(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (lcp_clsO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(OBJ_DOOR_CLOSET_CLOSED, 75, 87);
                gameTick(2);
                od_draw(OBJ_DOOR_CLOSET_OPEN_1, 75, 87);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(OBJ_DOOR_CLOSET_OPEN_2, 75, 87);
                gameTick(2);
                lcp_clsO = YES;
        }

        /* Walk into the closet. */
        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 87;

        hs_posXY(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 3;
        g_wtx = g_wtx - 10;
        g_actif = YES;
        lcp_wkD();
        saved_x = lcp_x;
        g_actif = NO;

        /* Close door behind: wide -> ajar -> lcp-inside. */
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_AJAR);
        g_sepex[g_seslm[SPRITE_CLOSET_AJAR]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_AJAR]] = 87;
        od_draw(OBJ_DOOR_CLOSET_OPEN_1, 75, 87);
        gameTick(1);

        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_LCP_INSIDE);
        hideLcp();
        g_sepex[g_seslm[SPRITE_CLOSET_LCP_INSIDE]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_LCP_INSIDE]] = 87;
        od_draw(OBJ_DOOR_CLOSET_CLOSED, 75, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(1);

        counter = rndRng(45, 60);
        gameTick(counter);
        if (introSeq == NO) {
                if (value == 0)
                        pa_cloc();
                else
                        pa_skic();
        }

        /* Open door back up + walk out. */
        g_selaf[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_AJAR);
        showLcp();
        g_sepex[g_seslm[SPRITE_CLOSET_AJAR]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_AJAR]] = 87;
        od_draw(OBJ_DOOR_CLOSET_OPEN_1, 75, 87);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 87;
        od_draw(OBJ_DOOR_CLOSET_OPEN_2, 75, 87);
        gameTick(1);
        lcp_clsO = YES;

        lcp_x = saved_x;
        hs_posXY(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        if (lcp_clsO != NO) {
                g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
                sp_upds();
                gameTick(0);
        }

        counter = rndRng(0, 100);
        if (lcp.initiative_threshold < counter ||
            introSeq != NO)
                a_clocd();
}

/* a_opcuc: walk to study door, 3-frame open if closed, enter study.
   Chains into lcp_std; value != 0 -> do_save=YES.
   addr: a_opcuc() */

void
a_opcuc(value)
short   value;
{
        short   result;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_face   = FACING_RIGHT;
        lcp_st              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();

        if (studyDrO == NO) {
                lcp_face = FACING_LEFT;
                lcp_st = STATE_BEND_AND_REACH;
                gameTick(2);
                od_draw(OBJ_DOOR_STUDY_CLOSED,  178, 23);
                gameTick(2);
                od_draw(OBJ_DOOR_STUDY_OPEN_1,  178, 23);
                sf_sele(SFX_DOOR_OPEN, 6L);
                gameTick(2);
                od_draw(OBJ_DOOR_STUDY_OPEN_2,  178, 23);
                gameTick(2);
                studyDrO = YES;
        }

        /* Walk into the study, ducking behind the wide-open door. */
        lcp_face = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;

        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 3;
        g_wtx = g_wtx - 10;
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        /* Swap wide-open sprite for ajar and hide the resident. */
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] =  23;
        od_draw(OBJ_DOOR_STUDY_OPEN_1, 178, 23);
        hideLcp();
        gameTick(1);
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();

        /* Continue into the study; value != 0 -> save HYBER. */
        if (value == 0)
                lcp_std(NO,  YES);
        else
                lcp_std(YES, YES);
}
