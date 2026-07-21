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

/* st_titl (Ghidra 0x16de6): title screen name/date/time/AM-PM entry.
   Phases: NAME (<= 18 upper chars), DATE (MM/DD/YY), TIME (HH:MM),
   AM/PM (adjusts t_hour to 24h).  Ends with 1s evnt_timer.
   Build switch -DSKIP_TITLE=1 keeps default PLAYER/noon/0-0-0 so
   test harnesses under --fast-forward don't block on getKey. */


#ifdef SKIP_TITLE
void
st_titl()
{
        short   i;
        /* Prime g_dscp: without this, prCh->Setscreen(NULL,...) triggers
           TOS v_gtext $fd330c fault before first fillTopR(). */
        g_dscp = sv_phb;

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
#else

/* drwCurs: 8x8 solid rect at (x, y-7)..(x+7, y) for cursor blink.
   addr: drwCurs() */

void
drwCurs(x, y, color)
short   x;
short   y;
short   color;
{
        drwBar(x, y - 7, x + 7, y, color);
}

/* inpNum: read `val` digits into in_str[] with blinking cursor.
   (i % 3 == 2) skip preserves the '/' or ':' separator.  Stored as
   raw 0..9 (ch - 0x30) so st_titl can decode tens*10 + ones.
   addr: inpNum() */

void
inpNum(x, y, str, val, color)
short   x;
short   y;
char *  str;
short   val;
short   color;
{
        short   ch;
        short   i;
        short   next;

        drwBar(x, y - 7, x + val * 8, y, COLOR_dk_brown);
        strPr(str, x, y, color);
        i = 0;
        do {
                do {
                        for (;;) {
                                ch = getKey();
                                if (ch != KEY_CURSOR_LEFT || i < 1)
                                        break;
                                next = i - 1;
                                if ((short)(i - 1) % 3 == 2)
                                        next = i - 2;
                                i = next;
                                drwCurs(
                                             x + i * 8, y, COLOR_dk_brown);
                                prCh((short) str[i],
                                                 x + i * 8, y, color);
                        }
                } while ((short) ch < '0' || '9' < (short) ch);
                drwCurs(x + i * 8, y, COLOR_dk_brown);
                prCh(ch, x + i * 8, y, color);
                in_str[i] = (char) ch - 0x30;
                next = i + 1;
                if ((short)(i + 1) % 3 == 2)
                        next = i + 2;
                i = next;
        } while (i < val);
}

void
st_titl()
{
        short   ch;
        short   parsed;
        short   ilen;
        short   xpos;
        short   pmc;   /* AM/PM char to display */

        /* g_dscp = sv_phb: redirect prCh's Setscreen at visible
           physbase, not the dsb_stor letter buffer left by stpScrB. */
        g_dscp = sv_phb;

        /* Decompress title.scn to physbase.  unScn folds Ghidra's
           inline fOpen+Malloc+read+decompress+Mfree into one call. */
        unScn("title.scn", (unsigned short *) sv_phb, 16000L);

        /* NAME phase. */
        strPr("NAME: ------------------", 80, 110, COLOR_lt_brown);
        xpos = 0;
        do {
                do {
                        for (;;) {
                                ch = getKey();
                                if (ch != KEY_CURSOR_LEFT || xpos <= 0)
                                        break;
                                xpos = xpos - 1;
                                drwCurs(
                                             xpos * 8 + 128, 110, COLOR_dk_brown);
                                prCh('-', xpos * 8 + 0x80, 110,
                                                 COLOR_lt_brown);
                        }
                        if (ch == KEY_CTRL_M && xpos > 0)
                                goto name_done;
                        ch = lcp_upp(ch);
                } while (ch < 0x20);
                lcp.owner_name[xpos] = (char) ch;
                drwCurs(xpos * 8 + 0x80, 110,
                                                                          COLOR_dk_brown);
                prCh(ch, xpos * 8 + 0x80, 110, COLOR_lt_brown);
                xpos = xpos + 1;
        } while (xpos != 0x12);
name_done:
        lcp.owner_name[xpos] = '\0';
        for (ilen = xpos; ilen < 18; ilen = ilen + 1)
                drwCurs(ilen * 8 + 128, 110,
                                                                          COLOR_dk_brown);

        /* DATE phase. */
        strPr("ENTER DATE:", 80, 122, COLOR_lt_brown);
        do {
                do {
                        inpNum(176, 122, "MM/DD/YY", 8,
                                                    COLOR_lt_brown);
                        dt_mon   = (short) in_str[1] + in_str[0] * 10 - 1;
                        date_day = (short) in_str[4] + in_str[3] * 10 - 1;
                        dt_year  = (short) in_str[7] + in_str[6] * 10;
                } while (dt_mon < 0);
        } while (dt_mon > 0xb || date_day < 0 ||
                 (parsed = daysInMo(dt_mon, dt_year),
                  parsed <= date_day));

        /* TIME phase. */
        strPr("ENTER TIME:", 80, 134, COLOR_lt_brown);
        do {
                do {
                        inpNum(176, 134, "HH:MM", 5,
                                                    COLOR_lt_brown);
                        t_hour = (short) in_str[1] + in_str[0] * 10;
                        t_min  = (short) in_str[4] + in_str[3] * 10;
                } while (t_hour == 0);
        } while (t_hour > 12 || t_min > 59);

        /* AM/PM phase. */
        strPr("AM OR PM: -M", 80, 146, COLOR_lt_brown);
        for (;;) {
                ch = getKey();
                if (ch == 'A' || ch == 'a') {
                        pmc = 'A';
                        if (t_hour == 12)
                                t_hour = 0;
                        break;
                }
                if (ch == 'P' || ch == 'p') {
                        pmc = 'P';
                        if (t_hour != 12)
                                t_hour = t_hour + 12;
                        break;
                }
        }
        drwCurs(160, 146, COLOR_dk_brown);
        prCh(pmc, 160, 146, COLOR_lt_brown);
        evnt_timer(1000, 0);
}
#endif   /* SKIP_TITLE */

/* mq_tick lives in mq_tick.s (privileged move-sr, rte). */

/* mq_intim (Ghidra 0x11112): install Timer-A for MIDI sequencer.
   addr: mq_intim() */

void
mq_intim()
{
#ifdef SKIP_MIDI
        /* SKIP_MIDI: Timer-A jitter breaks frame-hash goldens under
           --fast-forward.  Interactive builds get the real handler. */
        (void) 0;
#else
        g_mtpre = 100;
        g_mtdiv = 4;
        mi_svtv = Setexc(0x4d, -1L);
        Xbtimer(0, 5, 0x28, (long) mq_tick);
#endif
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
        od_draw(OBJ_DOOR_FRONT_OPEN_1, 294, 151);
        sf_sele(SFX_DOOR_OPEN, 6);
        gameTick(2);
        od_draw(OBJ_DOOR_FRONT_OPEN_2, 294, 151);
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
#ifdef TEST_STAIRS
                /* -DTEST_STAIRS=1 descend attic->bottom, 2 ascend.
                   chk_actT ignores externally-set g_wtx/g_wty so we
                   call lcp_wkD directly. */
                {
#if TEST_STAIRS == 1
                        lcp_x = 182; lcp_y = 72;
                        g_wtx = 300; g_wty = 195;
#else
                        lcp_x = 170; lcp_y = 185;
                        g_wtx = 300; g_wty = 45;
#endif
                        g_wyx = 0; g_wyy = 0;
                        lcp_stR = NO;
                        lcp_wkD();
                }
#endif
                return;
        }
        for (;;) a_sleep(-1);
}
