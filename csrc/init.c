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
extern short    g_mtpre;
extern short    g_mtdiv;
extern long     mi_svtv;

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

/* st_titl -- full port of Ghidra show_title_screen_enter_name_and_date
   (Ghidra 0x16de6).  Decompresses title.scn to the visible physbase,
   then walks four interactive input phases:
     1. NAME -- up to 18 uppercased chars, cursor-left backspace,
                     Enter finishes early (min 1 char)
     2. DATE -- MM/DD/YY, re-prompt on invalid month/day
     3. TIME -- HH:MM, re-prompt on hours == 0 or > 12 or minutes > 59
     4. AM/PM -- adjusts t_hour to 24-hour internal representation
   Ends with a 1-second evnt_timer pause before returning.

   Build-time switch:
     -DSKIP_TITLE=1    ->  keep the pre-existing PLAYER/noon/0-0-0
                                 defaults path, no visual + no getKey blocking.
                                 Used by frame_hash / test_stairs etc. that
                                 must run under Hatari's --fast-forward.
     default           ->  full 1985 behaviour (visual + interactive) */

extern short    date_day;
extern short    dt_mon;
extern short    dt_year;
extern short    t_hour;
extern short    t_min;
extern void *   sv_phb;
extern void *   g_dscp;
extern void     unScn();
extern char     in_str[];
extern void     prCh();
extern void     drwBar();
extern void     strPr();
extern short    getKey();
extern short    lcp_upp();
extern short    daysInMo();
extern void     draw_text_input_cursor_8x8();
extern void     string_input();

#ifdef SKIP_TITLE
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
#else

/* draw_text_input_cursor_8x8: paint an 8x8 solid rect at (x, y-7)..
   (x+7, y).  Called alternately with COLOR_dk_brown (erase) and the
   text colour (rewrite) to blink the cursor between characters.
   addr: draw_text_input_cursor_8x8() */

void
draw_text_input_cursor_8x8(x, y, color)
short   x;
short   y;
short   color;
{
        drwBar(x, y - 7, x + 7, y, color);
}

/* string_input: read `val` digits into in_str[] with a blinking
   cursor.  Only digits 0..9 are accepted; cursor-left erases the
   most recent digit.  The (i % 3 == 2) skip-past-separator pattern
   keeps the '/' in "MM/DD/YY" or ':' in "HH:MM" from being
   overwritten during input.  Digits are stored as raw 0..9 values
   (ch - 0x30) in in_str[], matching Ghidra's post-parse arithmetic
   in st_titl (date/time decoded as tens*10 + ones).
   addr: string_input() */

void
string_input(x, y, str, val, color)
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
                                draw_text_input_cursor_8x8(
                                             x + i * 8, y, COLOR_dk_brown);
                                prCh((short) str[i],
                                                 x + i * 8, y, color);
                        }
                } while ((short) ch < '0' || '9' < (short) ch);
                draw_text_input_cursor_8x8(x + i * 8, y, COLOR_dk_brown);
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

        /* Ghidra's first statement: dest_screenbase_ptr = save_physbase.
           Required so prCh's Setscreen(g_dscp, ...) redirects VDI text
           output at visible physbase instead of the dsb_stor letter
           buffer stpScrB left g_dscp pointing at.  Missing this was a
           port literal-audit hole -- text on the title screen was
           silently going to the offscreen letter buffer. */
        g_dscp = sv_phb;

        /* Decompress title.scn straight to visible physbase.  Port's
           unScn folds Ghidra's inline fOpen + Malloc + read +
           decompress + Mfree into one call. */
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
                                draw_text_input_cursor_8x8(
                                             xpos * 8 + 128, 110, COLOR_dk_brown);
                                prCh('-', xpos * 8 + 0x80, 110,
                                                 COLOR_lt_brown);
                        }
                        if (ch == KEY_CTRL_M && xpos > 0)
                                goto name_done;
                        ch = lcp_upp(ch);
                } while (ch < 0x20);
                lcp.owner_name[xpos] = (char) ch;
                draw_text_input_cursor_8x8(xpos * 8 + 0x80, 110,
                                                                          COLOR_dk_brown);
                prCh(ch, xpos * 8 + 0x80, 110, COLOR_lt_brown);
                xpos = xpos + 1;
        } while (xpos != 0x12);
name_done:
        lcp.owner_name[xpos] = '\0';
        for (ilen = xpos; ilen < 18; ilen = ilen + 1)
                draw_text_input_cursor_8x8(ilen * 8 + 128, 110,
                                                                          COLOR_dk_brown);

        /* DATE phase. */
        strPr("ENTER DATE:", 80, 122, COLOR_lt_brown);
        do {
                do {
                        string_input(176, 122, "MM/DD/YY", 8,
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
                        string_input(176, 134, "HH:MM", 5,
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
        draw_text_input_cursor_8x8(160, 146, COLOR_dk_brown);
        prCh(pmc, 160, 146, COLOR_lt_brown);
        evnt_timer(1000, 0);
}
#endif   /* SKIP_TITLE */

/* Forward decl for the timer handler installed by mq_intim.
   mqisr is the asm wrapper (mq_hlpr.s) around the C mq_tick;
   it save/restores scratch registers and RTEs.  Xbtimer needs the
   wrapper because K&R C returns via RTS. */
extern void     mq_tick();
extern void     mqisr();
extern void     mq_advs();      /* midi_seq_advance_sequencer (skeleton) */
extern void     psg_upEn();     /* psg_process_envelopes    (skeleton) */
extern BOOL16   psg_ntAc;

/* mq_intim (Ghidra 0x11112): install the Timer-A interrupt for the
   MIDI sequencer.  Ghidra body:
     midi_tick_prescaler   = 100
     midi_tick_divider     = 4
     midi_saved_timer_vect = Bios(Setexc, 0x4d, -1)   (query only)
     Xbtimer(0, 5, 0x28, midi_seq_tick_handler)

   Port status: partially ported.  The tick counter, prescaler,
   divider, and vector-save are done here.  The Xbtimer install is
   NOT done yet because it needs an assembly wrapper -- a K&R C
   function returns via RTS, but MFP interrupt handlers must return
   via RTE, so passing mq_tick directly to Xbtimer causes the CPU
   to pop the wrong stack frame and jump to garbage.  Fix requires
   a small .s file with:
       _mq_tick_asm:
             movem.l  D0-D2/A0-A2, -(SP)
             jsr      _mq_tick
             movem.l  (SP)+, D0-D2/A0-A2
             rte
   and passing `_mq_tick_asm` to Xbtimer instead.  Deferred to a
   follow-up commit; verified by isolating the crash to exactly
   the xbios(31, ...) call.

   The `bios(Setexc, 0x4d, -1)` query is safe on its own (no side
   effects), so it stays.  The mq_tick / mq_advs / psg_upEn C
   bodies are in place -- once the asm wrapper lands, they'll
   start driving the sequencer.

   addr: mq_intim() */

void
mq_intim()
{
#ifdef SKIP_MIDI
        /* Automated tests set -DSKIP_MIDI=1 to keep runs deterministic:
           the 200 Hz Timer-A interrupt fires at times that shift by a
           few cycles between Hatari boots, and the game loop's state
           at a fixed VBL count then differs enough to break
           warp-based stair tests + frame-hash goldens.  Interactive
           builds get the real handler. */
        (void) 0;
#else
        g_mtpre = 100;
        g_mtdiv = 4;
        mi_svtv = bios(BIOS_Setexc, 0x4d, -1L);
        xbios(31, 0, 5, 0x28, (long) mqisr);
#endif
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

#ifdef TEST_ACTIONS
                /* Enqueue one test event so chk_actT dispatches it as
                   soon as gameLoop takes over.  Guarded by
                   -DTEST_ACTIONS=<id> in the test build. */
                {
                        extern void putEv();
                        putEv(TEST_ACTIONS);
                }
#endif
#ifdef TEST_KEY
                /* Invoke the keyboard dispatcher with a single keycode
                   to exercise the Ctrl-letter / cursor / printable
                   paths.  Guarded by -DTEST_KEY=<code> in the test
                   build. */
                {
                        extern void deal_kc();
                        deal_kc(TEST_KEY);
                }
#endif
                return;
        }
        for (;;) a_sleep(-1);
}
