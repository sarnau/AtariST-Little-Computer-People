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

/* initBRev (Ghidra 0x16804): thin wrapper around
   build_bit_revert_table which fills rev_tab[256] with bit-
   reversed byte values.  Port has rev_tab as a static-initialised
   constant, so this is a no-op semantic-equivalent (the runtime table
   contents match Ghidra's post-init state).  Kept as a distinct entry
   point so main() can call it at the exact Ghidra step. */

void
initBRev()
{
        /* rev_tab is already static-init in tables.c.  Ghidra's
           runtime build produces the same 256 entries.  Nothing to do. */
}

/* cs_mvIn: minimal replacement for the doorbell/
   door-open/room-tour cutscene in Ghidra.  We SKIP the tour animation
   but reproduce the exit state so the AI loop starts from valid
   positions:
     lcp at (300, 190)   -- right side of ground floor
     lcp_st = STAND_SIDE_VIEW, facing right
     head anim initialised so sp_lcha doesn't loop
     dog at (273, 190), initial wander target seeded
     introSeq released so the event queue can drain.
   TODO: port the full cutscene once the AI-loop path is stable. */

void
cs_mvIn()
{
        lcp_x                     = 300;
        lcp_y                     = 190;
        lcp_face      = FACING_RIGHT;
        lcp_st                 = STATE_STAND_SIDE_VIEW;
        g_hatas                   = 8;
        g_hacur                   = 8;
        g_hamod                   = HEAD_ANIM_DISABLED;

        dog_x                     = 273;
        dog_y                     = 190;
        g_dtx                     = 0;
        g_dty                     = 0;
        g_dyx                     = 0;
        g_dyy                     = 0;
        dg_stair        = NO;
        dg_idlcd        = 20;
        dg_ltgtI     = g_dgitx;
        dg_init           = NO;

        /* Push initial dog sprite (lay-down pose) into the dog slot. */
        sp_spud(SPRITE_DOG_LAY_DOWN, -1, YES);

        introSeq     = NO;

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
}
