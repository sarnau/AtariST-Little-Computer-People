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


/* lcp_crnd (Ghidra 0x169D8): populate PLAYER for a new game.
   1985 code also picks a random name from "names"; skipped here
   (avoids fOpen); character_name left NUL. */

void
lcp_crnd()
{
        lcp.character_sprite_id       = rndRng(2, 6);
        lcp.character_name[0]         = 0;
        lcp.water_level               = 7;
        lcp_watr               = 7;
        lcp.clothing_color            = rndRng(0, 15);
        lcp.skin_color                = rndRng(0, 7);
        lcp.bedtime_hour              = rndRng(22, 24);
        if (lcp.bedtime_hour > 23)
                lcp.bedtime_hour = lcp.bedtime_hour - 24;
        lcp.wake_hour                 = lcp.bedtime_hour + 6;
        if (lcp.wake_hour > 23)
                lcp.wake_hour = lcp.bedtime_hour - 18;
        lcp.lunch_hour                = rndRng(11, 13);
        lcp.dinner_hour               = rndRng(17, 19);
        lcp.personality_type          = rndRng(0, 3);
        lcp.activity_level            = rndRng(0, 7);
        lcp.happiness                 = MOOD_CONTENT;
        lcp.happiness_initial_countdown = rndRng(6, 24);
        lcp.happiness_duration_happy    = rndRng(6, 24);
        lcp.happiness_duration_content  = rndRng(6, 12);
        lcp.happiness_duration_active   = lcp.happiness_duration_happy;
        lcp.happiness_direction       = -1;             /* DIR_IMPROVING */
        lcp.sickness_level            = 0;              /* SICKNESS_HEALTHY */
        lcp.sickness_countdown        = 0;
        lcp.sickness_direction        = 0;              /* DIR_STABLE */
        lcp.is_sleeping               = NO;
        lcp.initiative_threshold      = rndRng(20, 80);
        lcp.thirst_level              = 0;              /* NEED_SATISFIED */
        lcp.thirst_timer_max          = rndRng(45, 75);
        lcp.thirst_timer              = lcp.thirst_timer_max;
        lcp.hunger_level              = 0;
        lcp.hunger_timer_max          = rndRng(75, 120);
        lcp.hunger_timer              = lcp.hunger_timer_max;
        lcp.bathroom_need             = NO;
        lcp.bathroom_timer_max        = rndRng(20, 40);
        lcp.bathroom_timer            = lcp.bathroom_timer_max;
        lcp.record_playing            = NO;
        lcp_recP            = NO;
        lcp.tv_on                     = NO;
        lcp_tv                     = NO;
        lcp.food_supply               = 4;
        lcp_food                = 4;
        lcp.door_states_and_flags     = 0x0800;         /* DSF_INIT_FOOD_FULL */
}

/* cl_drini (Ghidra 0x233B4): paint clock-face center, cl_redrH. */

void
cl_drini()
{
        drwLine(278, 83, 281, 83, COLOR_white);
        cl_redrH();
}

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

/* mq_intim (ROM 0x804e): an EMPTY stub.  This binary never installs
   a Timer-A ISR -- there is no Xbtimer call anywhere in the ROM; the
   Timer-A machinery (mq_tick.s) came from the other Ghidra image.
   addr: mq_intim() */

void
mq_intim()
{
}

/* cntSong: enumerate *.SNG and *.ORG, count into sng_cnt / org_cnt.
   addr: Ghidra count_songs (main step 8). */


void
cntSong()
{
        short   result;
        long    next;

        sng_cnt = 0;
        org_cnt = 0;
        result = (short) Fsfirst("*.sng", 0L);
        if (result == 0) {
                sng_cnt = 1;
                for (;;) {
                        next = Fsnext();
                        if (next != 0) break;
                        sng_cnt = sng_cnt + 1;
                }
        }
        result = (short) Fsfirst("*.org", 0L);
        if (result == 0) {
                org_cnt = 1;
                for (;;) {
                        next = Fsnext();
                        if (next != 0) break;
                        org_cnt = org_cnt + 1;
                }
        }
}

/* bldBRev (Ghidra 0x1680e): fill rev_tab[256] with bit-reversed bytes. */


void
bldBRev()
{
        unsigned short  v;
        short           j;
        unsigned short  i;
        unsigned short *ptr;

        ptr = rev_tab;
        for (i = 0; (short) i < 0x100; i = i + 1) {
                v = 0;
                for (j = 0; j < 8; j = j + 1) {
                        if ((bm_msb_lsb[j] & i) != 0)
                                v = bm_lsb_msb[j] | v;
                }
                *ptr = v;
                ptr = ptr + 1;
        }
}

/* initBRev (Ghidra 0x16804): wrapper for bldBRev, kept for boot-step parity. */

void
initBRev()
{
        bldBRev();
}

/* a_chfd: resident checks the front door for `wait_ticks` frames,
   randomly closes based on initiative_threshold.
   addr: action_check_front_door() */


void
a_chfd(wait_ticks)
short   wait_ticks;
{
        short   result;

        hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
        result = lcp_wkD();
        if (result != 0)
                return;

        lcp_face = FACING_RIGHT;
        lcp_st   = STATE_STAND_FACING_SCREEN;
        g_hatas  = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_hwt();
        if (lcp_frdO == NO)
                a_opcfd(0);

        g_actif = YES;
        hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_updb(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;
        hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(wait_ticks);
        showLcp();
        hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

        result = rndRng(0, 100);
        if (lcp.initiative_threshold < result) {
                g_actif = YES;
                hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face = FACING_RIGHT;
                lcp_st   = STATE_STAND_FACING_SCREEN;
                g_hatas  = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(1);
        }
        g_actif = NO;
}

/* cs_mvIn: "resident moves in" cutscene (first run only).
   Doorbell x2, opens door, resident walks in from right, scripted
   room tour (if copy-protection passed).
   addr: cutscene_new_lcp_move_in() */


void
cs_mvIn()
{
        dg_init  = YES;
        introSeq = YES;
        hideLcp();
        gameTick(0xf0);
        p_dobls();
        gameTick(0x50);
        p_dobls();
        gameTick(0x18);
        od_draw(od_fro1, 294, 151);
        sf_sele(SFX_DOOR_OPEN, 6);
        gameTick(2);
        od_draw(od_fro2, 294, 151);
        gameTick(2);
        lcp_frdO = YES;
        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_updb(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;
        lcp_x = 300;
        lcp_y = 190;
        showLcp();
        hs_posXY(POS_BTM_SCREEN_EDGE, &g_wtx, &g_wty);
        g_wtx = g_wtx - 50;
        lcp_wkD();
        lcp_st = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();
        gameTick(0x10);

        if (cprot_r != 0) {
                hs_posXY(POS_BTM_KITCHEN_CABINET, &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face = FACING_RIGHT;
                lcp_st   = STATE_STAND_FACING_SCREEN;
                g_hatas  = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opecc(0);
                gameTick(0x10);
                a_opecc(1);
                hs_posXY(POS_BTM_KITCHEN_SINK, &g_wtx, &g_wty);
                lcp_wkD();
                gameTick(8);
                a_gesff();
                tt_on();
                lcp_st  = STATE_STAND_SIDE_VIEW;
                g_hatas = 8;
                lcp_hwt();
                a_getd();
                a_opcuc(0);
                a_wakum();
                hs_posXY(POS_MID_DRESSER, &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face = FACING_RIGHT;
                lcp_st   = STATE_STAND_FACING_SCREEN;
                g_hatas  = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcbc(0);
                a_uset();
                hs_posXY(POS_MID_BATHROOM_SINK, &g_wtx, &g_wty);
                lcp_wkD();
                a_gesff();
                a_playc();
                a_tidyh();
                a_wandi();
                tt_off();
                a_chfd(100);
                wkFrDr();
                lcp_face = FACING_RIGHT;
                lcp_st   = STATE_STAND_FACING_SCREEN;
                g_hatas  = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_hwt();
                a_opcfd(0);
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_REACH_FORWARD; gameTick(2);
                lcp_st = STATE_BEND_DOWN;    gameTick(1);
                lcp_st = STATE_STAND_FACING_SCREEN; gameTick(0);
                sp_ssco(SPRITE_SUITCASE);
                hs_posXY(POS_MID_DRESSER, &g_wtx, &g_wty);
                lcp_wkD();
                lcp_face = FACING_RIGHT;
                lcp_st   = STATE_STAND_FACING_SCREEN;
                g_hatas  = HEAD_ANIM_HORIZONTAL_RANGE;
                g_selaf[SPRITE_SUITCASE] = SPRITE_HIDDEN;
                sp_upds();
                g_lcyof = NO;
                lcp_hwt();
                a_opecd(0);
                hs_posXY(POS_BTM_FRONT_DOOR, &dog_x, &dog_y);
                dog_y = 190;
                dog_x = 273;
                dg_ltgtI = g_dgitx;
                hs_posXY(g_dgitx, &g_dtx, &g_dty);
                g_dyy = g_dgiyo + g_dty;
                g_dyx = g_dtx;
                dg_stair = NO;
                dg_idlcd = 20;
                dg_init  = NO;
                g_dty    = g_dyy;
                sp_spud(SPRITE_DOG_LAY_DOWN, -1, YES);
                a_opcbc(0);
                a_opcuc(1);
                introSeq = NO;

#ifdef TEST_ACTIONS
                /* -DTEST_ACTIONS=<id>: enqueue one test event. */
                {
                        putEv(TEST_ACTIONS);
                }
#endif
#ifdef TEST_KEY
                /* -DTEST_KEY=<code>: dispatch one keycode. */
                {
                        deal_kc(TEST_KEY);
                }
#endif
                return;
        }
        for (;;) a_sleep(-1);
}
