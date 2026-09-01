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

/* dbg_prA (ROM 0x8030): dead debug helper the ROM shipped between
   st_titl and mq_intim -- draws 'A' at (100,8) in colour 0.  Nothing
   calls it; kept for byte-identity of init.o. */

void
dbg_prA()
{
        prCh(65, 100, 8, 0);
}

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

/* cntSong: enumerate *.SNG and *.ORG, count into sng_cnt / org_cnt.
   addr: Ghidra count_songs (main step 8). */


void
cntSong()
{
        short   result;
        long    next;

        sng_cnt = 0;
        org_cnt = 0;
        /* ROM shapes: one trailing 0L on Fsfirst, bare Fsnext. */
        result = (short) gemdos(0x4E, "*.sng", 0L);
        if (result == 0) {
                sng_cnt = 1;
                for (;;) {
                        next = gemdos(0x4F);
                        if (next != 0) break;
                        sng_cnt = sng_cnt + 1;
                }
        }
        result = (short) gemdos(0x4E, "*.org", 0L);
        if (result == 0) {
                org_cnt = 1;
                for (;;) {
                        next = gemdos(0x4F);
                        if (next != 0) break;
                        org_cnt = org_cnt + 1;
                }
        }
}


/* initBRev (ROM 0x80fe): an empty stub in the ROM -- rev_tab ships
   as initialized data (tables.c); nothing to build at runtime. */

void
initBRev()
{
}


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
