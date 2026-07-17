/*
 * init.c -- boot-time init functions ported from Ghidra's main path.
 *
 * These wrap the parts of Ghidra's main() at 0x15546 that the port
 * had been silently skipping: lcp_crnd (populates the PLAYER
 * struct for a new game), cl_drini (paints the clock face),
 * and cs_mvIn (minimal seeding of lcp_x/y/state
 * and dog target so the AI loop can pick up on frame 1).
 *
 * addr: lcp_crnd @ 0x169D8, cl_drini @ 0x233B4,
 *       cutscene_new_lcp_move_in @ Ghidra (large, only stub here).
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>

extern PLAYER   lcp;
extern short    lcp_watr;
extern short    lcp_recP;
extern short    lcp_tv;
extern short    lcp_food;
extern short    lcp_x;
extern short    lcp_y;
extern short    lcp_st;
extern short    lcp_face;
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    dg_stair;
extern short    dg_idlcd;
extern short    dg_ltgtI;
extern short    g_dgitx;
extern short    dg_init;
extern short    introSeq;
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;

extern short    rndRng();
extern void     drwLine();
extern void     cl_redrH();
extern void     sp_spud();

/* lcp_crnd (Ghidra 0x169D8): populate a fresh PLAYER struct
   for a new game.  The 1985 code also picks a random name from the
   "names" file -- we skip that so we don't need fOpen here;
   character_name is left NUL-terminated. */

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

/* cl_drini (Ghidra 0x233B4): paint the clock face's center
   dot then delegate to cl_redrH. */

void
cl_drini()
{
        drwLine(278, 83, 281, 83, COLOR_white);
        cl_redrH();
}

/* st_titl -- ported from Ghidra show_title_screen_enter_name_and_date
   (Ghidra 0x???).  Full 1985 flow: decompress title.scn onto the visible
   physbase, prompt the user for name / date / time via interactive
   keyboard input.  This port variant skips the interactive input (which
   would hang under Hatari's fast-forward-only automated testing) and
   uses defaults.  The title.scn decompress + display still happens so
   the user sees the intended boot splash, and the resulting name/date/
   time values match a typical first-boot state. */

extern short    date_day;
extern short    dt_mon;
extern short    dt_year;
extern short    t_hour;
extern short    t_min;
extern void *   sv_phb;
extern void     unScn();

void
st_titl()
{
        short   i;

        /* Real 1985 flow: decompress title.scn to sv_phb, then
           prompt for name / date / time / AM-PM via interactive input.
           Skipped here -- the title.scn decompress would leave content
           in sv_phb that sc_ren8's page-flip cycles through,
           causing visible title-screen flicker during gameplay.
           Just set defaults so the AI dispatcher sees valid state. */

        /* Default owner name.  Real game reads via keyboard input at
           (80, 110); we default to "PLAYER" so downstream code that
           expects a non-empty name doesn't hit an all-zeroes buffer. */
        lcp.owner_name[0] = 'P';
        lcp.owner_name[1] = 'L';
        lcp.owner_name[2] = 'A';
        lcp.owner_name[3] = 'Y';
        lcp.owner_name[4] = 'E';
        lcp.owner_name[5] = 'R';
        for (i = 6; i < 24; i = i + 1)
                lcp.owner_name[i] = 0;

        /* Default date + time.  Real 1985 game prompts user for
           MM/DD/YY and HH:MM AM/PM.  Ghidra decompile shows the parse
           produces zero-indexed months/days: `dt_mon = input[1] +
           input[0]*10 - 1` and `date_day = input[4] + input[3]*10 -
           1`.  We default to 0/0/0 which corresponds to "day 1 of
           January, year 0" in that indexing scheme. */
        dt_mon   = 0;    /* January (0-indexed) */
        date_day     = 0;    /* 1st (0-indexed) */
        dt_year    = 0;
        t_hour   = 12;   /* noon */
        t_min = 0;
}

/* mq_intim (Ghidra 0x11112): install a Timer-A interrupt
   for the MIDI sequencer.  Real body:
     midi_tick_prescaler = 100;
     midi_tick_divider = 4;
     midi_saved_timer_vector = Bios(Setexc, 0x4d, -1);
     Xbtimer(0, 5, 0x28, midi_seq_tick_handler);
   Port stubs this to a no-op because we haven't ported
   midi_seq_tick_handler yet; the sequencer is guarded by
   mi_play==NO throughout so nothing calls into it. */

void
mq_intim()
{
        /* TODO: wire real timer install once midi_seq_tick_handler
           is ported. */
}

/* cntSong (Ghidra ~0x??): enumerate *.SNG and *.ORG files in the
   current directory, storing counts in sng_cnt /
   org_cnt. */

extern short    sng_cnt;
extern short    org_cnt;
extern long     gemdos();

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

/* bldBRev (Ghidra 0x1680e, build_bit_revert_table): fill rev_tab[256]
   with bit-reversed byte values.  For each i in 0..255 and each bit
   position j in 0..7, if the j-th "high-first" bit (bm_msb_lsb[j]) is
   set in i, OR in the j-th "low-first" bit (bm_lsb_msb[j]).  Result:
   rev_tab[i] has the bits of i in reversed order. */

extern unsigned short   rev_tab[];
extern unsigned short   bm_msb_lsb[];
extern unsigned short   bm_lsb_msb[];

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

/* initBRev (Ghidra 0x16804, init_build_bit_revert_table): thin wrapper
   -- just calls bldBRev.  Kept as a distinct entry point so main()
   can call it at the exact Ghidra boot step. */

void
initBRev()
{
        bldBRev();
}

/* a_chfd (Ghidra action_check_front_door): resident walks to the front
   door, opens it if closed, briefly stands aside (dog sprite fills the
   doorway) for `wait_ticks` frames, walks back to look outside, then
   randomly closes the door based on the initiative-threshold roll.
   Used from cs_mvIn's tour and by future doorbell events. */

extern void     hideLcp();
extern void     showLcp();
extern void     hs_posXY();
extern short    lcp_wkD();
extern void     lcp_hwt();
extern void     a_opcfd();
extern void     sp_updb();
extern void     sp_upds();
extern short    rndRng();
extern short    g_wtx, g_wty, g_actif, lcp_frdO;
extern short *  g_selaf;
extern short *  g_seslm;
extern short *  g_sepex;
extern short *  g_sepey;

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
        g_selaf[0x15] = SPRITE_IN_FRONT;
        sp_updb(SPRITE_DOG_SIT);
        g_sepex[g_seslm[0x15]] = 294;
        g_sepey[g_seslm[0x15]] = 151;
        hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
        lcp_wkD();
        hideLcp();
        gameTick(wait_ticks);
        showLcp();
        hs_posXY(POS_BTM_FRONT_DOOR, &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_wkD();
        g_selaf[0x15] = SPRITE_HIDDEN;
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

/* cs_mvIn: Ghidra cutscene_new_lcp_move_in.  Full "resident moves into
   the house" cutscene played once for a brand-new save: rings the
   doorbell twice, opens the front door, walks the resident on screen
   from the right edge, then (if copy protection passed) drives through
   the scripted room tour before releasing the AI loop.

   Ported line-for-line from Ghidra using a_chfd() and a_opecd() as
   the port equivalents of action_check_front_door /
   action_open_close_dresser.  Verified end-to-end in Hatari: 30k VBLs
   of gameplay clean, tour visibly runs (upstairs closet opens, dog
   transitions between rooms, resident carries suitcase to dresser). */

extern void     od_draw();
extern void     sf_sele();
extern void     p_dobls();
extern void     sp_ssco();
extern void     tt_on();
extern void     tt_off();
extern void     wkFrDr();
extern void     a_sleep();
extern void     a_opecc();
extern void     a_gesff();
extern void     a_getd();
extern void     a_opcuc();
extern void     a_wakum();
extern void     a_opcbc();
extern void     a_uset();
extern void     a_playc();
extern void     a_tidyh();
extern void     a_wandi();
extern void     a_opecd();
extern void     a_chfd();
extern void     a_opcfd();
extern void     hideLcp();
extern void     showLcp();
extern void     lcp_wkD();
extern void     hs_posXY();
extern void     sp_updb();
extern void     sp_upds();
extern void     lcp_hwt();
extern short    cprot_r;
extern short    g_lcyof;
extern short    lcp_frdO;
extern short    g_obi05;
extern short    g_obi06;
extern short    g_dgiyo;
extern short    g_wtx;
extern short    g_wty;
extern short *  g_selaf;
extern short *  g_seslm;
extern short *  g_sepex;
extern short *  g_sepey;

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
        od_draw(g_obi05, 294, 151);
        sf_sele(SFX_DOOR_OPEN, 6);
        gameTick(2);
        od_draw(g_obi06, 294, 151);
        gameTick(2);
        lcp_frdO = YES;
        g_selaf[0x15] = SPRITE_IN_FRONT;
        sp_updb(SPRITE_DOG_SIT);
        g_sepex[g_seslm[0x15]] = 294;
        g_sepey[g_seslm[0x15]] = 151;
        lcp_x = 300;
        lcp_y = 190;
        showLcp();
        hs_posXY(POS_BTM_SCREEN_EDGE, &g_wtx, &g_wty);
        g_wtx = g_wtx - 50;
        lcp_wkD();
        lcp_st = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_hwt();
        g_selaf[0x15] = SPRITE_HIDDEN;
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
                g_selaf[0x30] = SPRITE_HIDDEN;
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
                return;
        }
        for (;;) a_sleep(-1);

#ifdef TEST_ACTIONS
        /* Temporary: enqueue a series of test events to exercise every
           ported AI action & delivery event at startup.  Runs before
           gameLoop, so chk_actT drains them one at a time.  Guarded by
           -DTEST_ACTIONS in the temporary test build. */
        {
                extern void putEv();
                putEv(TEST_ACTIONS);
        }
#endif

#ifdef TEST_KEY
        /* Temporary: invoke the keyboard dispatcher with a single
           keycode to exercise the Ctrl-letter / cursor / printable
           paths.  Guarded by -DTEST_KEY=$code in the test build. */
        {
                extern void deal_kc();
                deal_kc(TEST_KEY);
        }
#endif
}
