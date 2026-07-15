/*
 * aleisure.c -- music, fireplace, couch, exercise, and the
 *                      lightweight house-upkeep handlers.
 *
 * Grouped because they all wire into the same music / dog / couch /
 * closet subsystems -- porting them one at a time forces the same
 * externs into each file.
 *
 * addr: a_lists(), a_playp(),
 *       a_plawr(), a_lighf(),
 *       a_socwd(), a_sitae(),
 *       a_chefd(), a_cleau(),
 *       a_tidyh(), a_opcbc()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern BOOL16   intro_sequence_active;
extern short    g_trel[];
extern short    lcp_x;
extern short    lcp_y;
extern short    g_hatas;
extern short    g_hamod;
extern BOOL16   g_actif;
extern BOOL16   dog_pettable_flag;
extern short    g_wtx;
extern short    g_wty;
extern short    PLAYER_STATE_ARRAY[];
extern void     lcp_wait_head_reach_target();
extern void     game_tick_and_animate();
extern short    lcp_front_door_open;
extern short    lcp_study_door_open;
extern short    lcp_closet_door_open;
extern short    lcp_cabinet_open;
extern short    lcp_dresser_open;
extern short    lcp_toilet_door_open;
extern short    lcp_filing_cabinet_open;
extern short    lcp_food_count;
extern short    lcp_record_playing;
extern short    g_obids;
extern short    g_obi07;
extern short    g_obi08;
extern BOOL16   midi_is_playing;
extern BOOL16   g_rbact;
extern char *   midi_song_buffer;
extern short    org_song_file_count;
extern BOOL16   fire_active_flag;
extern short    fire_duration_countdown;
extern short    g_obidc;
extern short    g_obi03;
extern short    g_obi04;
extern long             g_momap;
extern BOOL16   g_ptdoa;
extern void     house_get_position_xy();
extern short    lcp_state;
extern short    lcp_facing_direction;
extern short    g_lcyof;
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    randomRange();                  /* random.c */
extern void     do_action();                    /* actions.c */
#include <osbind.h>

extern short    randomRange();
extern short    lcp_walk_to_destination();
extern void     sp_sprs();
extern void     sp_ssco();
extern void     sp_upds();
extern void     sf_sele();
extern void     od_draw();
extern void     a_opcfd();
extern void     a_opcfc();
extern void     a_opecc();
extern void     a_opecd();
extern void     a_clotd();
extern void     a_clocd();
extern void     a_watat();
extern void     a_calld();
extern void     a_playp();
extern void     hide_lcp_sprites();
extern void     show_lcp_sprites();
extern void     li_loor();
extern void     song_play();
extern void     mq_inis();
extern void     pa_cloc();
extern void     pa_skic();
extern void     walk_to_front_door();
/* host_gemdos_trap declared via _gemdos macro in osbind.h */
extern char     input_string[];         /* not used yet, letter subsystem */
extern void     lcp_enter_study_and_save();

/* Ghidra references. */
/* g_momap declared in globals.h */

/* DTA (Disk Transfer Address) layout used by GEMDOS Fsfirst/Fsnext.
   Only d_fname is read by the caller; the rest is padded. */
typedef struct {
        char    _reserved[30];
        char    d_fname[14];
} DTA;

/* a_lists: pick a random .sng file and start it playing.
   Uses lcp_food_count as a modulo index (yes, it's a bit hacky; the
   1985 code reused the field).

   .sng files on the LCP disk are byte-exact copies of Activision
   Music Studio 2.0 demo songs (AISLEDAN, BALLAD, BEBOP, BOSSA,
   CALYPSO, COUNTRY2, CANON, FIVEFOUR, MYSTERY, BOOGIE); the
   Music Studio disk shipped with the same files, so any tune the
   resident dances to originally came from Ed Bogas / Audio Light's
   1986 authoring tool.  See sound.c:song_play for the full file
   format details.
   addr: a_lists() */

void
a_lists()
{
        short   result;
        short   index;
        DTA *   dta_ptr;
        char *  filename;
        short   i;

        if (lcp_record_playing != NO)
                return;

        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        game_tick_and_animate(2);
        li_loor();
        lcp_record_playing = YES;

        index = randomRange(0, lcp_food_count - 1) + 1;
        _gemdos(GEMDOS_Fsfirst, (long) "*.sng", 0L, 0L);
        while ((index = index - 1) != 0)
                _gemdos(GEMDOS_Fsnext, 0L, 0L, 0L);
        dta_ptr = (DTA *) _gemdos(GEMDOS_Fgetdta, 0L, 0L, 0L);
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
        song_play(filename);
}

/* a_playp: shortcut used mostly to *stop* a currently-playing
   record so the resident can start writing / typing.  Walks to the
   dance floor, spins the midi buffer to end, frees it, clears the
   record-playing flag.
   addr: a_playp() */

void
a_playp()
{
        short   result;

        if (lcp_record_playing == NO)
                return;

        house_get_position_xy(POS_TOP_DANCE_FLOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        game_tick_and_animate(2);

        if (midi_is_playing != NO) {
                mq_inis(midi_song_buffer, g_momap);
                while (midi_is_playing != NO)
                        ;
        }
        li_loor();
        lcp_record_playing = NO;
        if (midi_song_buffer != (char *) 0) {
                _gemdos(GEMDOS_Mfree, (long) midi_song_buffer, 0L, 0L);
                midi_song_buffer = (char *) 0;
        }
}

/* a_plawr: browse the vinyl shelf and put on a random
   .org file.  The animation is amplitude-reactive: PSG channel volumes
   are polled via XBIOS Giaccess and if any channel got louder we
   randomise the browsing pose.  On the host the PSG stub returns 0
   forever so we fall through the amp check and hold the reach-right
   pose.

   .org files (FOLKSONG, MAPLE = Maple Leaf Rag, PRELUDE, REQUIEM,
   STARSPAN = Star-Spangled Banner) are cosmetically-renamed Music
   Studio 2.0 exports -- same file format as .sng, just organised
   into a classical/organ category for the vinyl-shelf UI.  PRELUDE
   and REQUIEM ship byte-identical to Music Studio's demo disk;
   STARSPAN is a shortened arrangement.  See sound.c:song_play.
   addr: a_plawr() */

void
a_plawr()
{
        short           result;
        DTA *           dta_ptr;
        char *          filename;
        long            xres;
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;

        PLAYER_STATE_ARRAY[0] = STATE_BROWSE_VINYL_REACH_RIGHT;
        PLAYER_STATE_ARRAY[1] = STATE_BROWSE_VINYL_IDLE;
        PLAYER_STATE_ARRAY[2] = STATE_BROWSE_VINYL_REACH_LEFT;
        PLAYER_STATE_ARRAY[3] = STATE_BROWSE_VINYL_PULL_OUT;

        prev_a = prev_b = prev_c = 0;
        g_actif = YES;
        if (lcp_record_playing != NO)
                a_playp();
        g_actif = NO;

        house_get_position_xy(POS_TOP_RECORD_SHELF,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        g_rbact = YES;
        g_hamod = HEAD_ANIM_DISABLED;
        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        game_tick_and_animate(4);

        lcp_state = STATE_BROWSE_VINYL_REACH_RIGHT;
        g_selaf[SPRITE_VINYL_RECORD] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_VINYL_RECORD);
        g_sepex[g_seslm[SPRITE_VINYL_RECORD]] = 146;
        g_sepey[g_seslm[SPRITE_VINYL_RECORD]] =  54;
        game_tick_and_animate(1);

        i = randomRange(1, org_song_file_count);
        _gemdos(GEMDOS_Fsfirst, (long) "*.org", 0L, 0L);
        while ((i = i - 1) != 0)
                _gemdos(GEMDOS_Fsnext, 0L, 0L, 0L);
        dta_ptr = (DTA *) _gemdos(GEMDOS_Fgetdta, 0L, 0L, 0L);
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i = i + 1)
                ;
        filename[i + 4] = '\0';
        song_play(filename);

        g_hamod = HEAD_ANIM_WALKING;
        while (midi_is_playing == NO)
                ;                        /* wait for playback to start */

        while (midi_is_playing != NO) {
                xres = Giaccess(0, 8);  psg_a = (unsigned char) xres & 0x1f;
                xres = Giaccess(0, 9);  psg_b = (unsigned char) xres & 0x1f;
                xres = Giaccess(0, 10); psg_c = (unsigned char) xres & 0x1f;

                lcp_state = PLAYER_STATE_ARRAY[0];
                if (prev_a < psg_a || prev_b < psg_b || prev_c < psg_c) {
                        i = randomRange(1, 3);
                        while (PLAYER_STATE_ARRAY[i] == lcp_state)
                                i = randomRange(1, 3);
                        lcp_state = PLAYER_STATE_ARRAY[i];
                        if (PLAYER_STATE_ARRAY[3] == lcp_state) {
                                game_tick_and_animate(0);
                                result = randomRange(1, 2);
                                lcp_state = PLAYER_STATE_ARRAY[result];
                        }
                }
                game_tick_and_animate(0);
                prev_c = psg_c; prev_b = psg_b; prev_a = psg_a;
        }

        g_hamod = HEAD_ANIM_DISABLED;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_state = PLAYER_STATE_ARRAY[0];
        lcp_wait_head_reach_target();
        game_tick_and_animate(8);

        lcp_state = STATE_STAND_FACING_SCREEN;
        g_selaf[SPRITE_VINYL_RECORD] = SPRITE_HIDDEN;
        sp_upds();
        game_tick_and_animate(0);

        if (midi_song_buffer != (char *) 0) {
                _gemdos(GEMDOS_Mfree, (long) midi_song_buffer, 0L, 0L);
                midi_song_buffer = (char *) 0;
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

        if (fire_active_flag != NO)
                return;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        a_opcfd(0);
        g_actif = YES;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_walk_to_destination();

        /* Sit-dog sprite waits at the porch. */
        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        hide_lcp_sprites();
        game_tick_and_animate(40);
        show_lcp_sprites();

        sp_ssco(SPRITE_FIREWOOD);
        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_walk_to_destination();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

        result = randomRange(0, 100);
        if (lcp.initiative_threshold < result)
                a_opcfd(1);

        house_get_position_xy(POS_BTM_FIREPLACE_LOGS,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_walk_to_destination();

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        g_selaf[SPRITE_FIREWOOD] = SPRITE_HIDDEN;
        sp_upds();
        g_lcyof = NO;
        lcp_wait_head_reach_target();

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_BEND_DOWN;      game_tick_and_animate(1);
        lcp_state = STATE_REACH_FORWARD;  game_tick_and_animate(1);
        lcp_state = STATE_STOKE_FIREPLACE;game_tick_and_animate(1);

        /* Random-direction shrug for 10 ticks so the resident looks
           like they're feeding kindling. */
        for (i = 0; i < 10; i = i + 1) {
                lcp_facing_direction = randomRange(0, 1);
                game_tick_and_animate(0);
        }

        fire_active_flag        = YES;
        fire_duration_countdown = randomRange(2500, 5000);

        lcp_facing_direction = FACING_RIGHT;
        lcp_state = STATE_STAND_FACING_SCREEN;
        game_tick_and_animate(0);
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
                lcp_state = STATE_STAND_SIDE_VIEW;
                game_tick_and_animate(0);
                return;
        }

        lcp_state = STATE_SIT_COUCH_UPRIGHT;
        lcp_y = lcp_y + 9;
        g_hatas = 8;
        lcp_wait_head_reach_target();
        game_tick_and_animate(3);

        lcp_y = lcp_y - 3;
        g_selaf[SPRITE_READING_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_READING_1);
        g_sepex[g_seslm[SPRITE_READING_1]] = 221;
        g_sepey[g_seslm[SPRITE_READING_1]] = 172;

        ticks = randomRange(30, 50);
        lcp_state = STATE_SIT_COUCH_PETTING_DOG;
        while (ticks != 0 && g_trel[0] == ACTION_NONE) {
                game_tick_and_animate(3);
                ticks = ticks - 1;
        }

        lcp_y = lcp_y + 3;
        lcp_state = STATE_SIT_COUCH_UPRIGHT;
        g_selaf[SPRITE_READING_1] = SPRITE_HIDDEN;
        sp_upds();
        g_hatas = 8;
        lcp_wait_head_reach_target();
        game_tick_and_animate(3);

        lcp_y = lcp_y - 9;
        lcp_state = STATE_CROUCH_DOWN;
        game_tick_and_animate(8);
        while (g_ptdoa != NO)
                game_tick_and_animate(0);

        dog_pettable_flag = NO;
        lcp_state = STATE_STAND_SIDE_VIEW;
        lcp_facing_direction = FACING_RIGHT;
        game_tick_and_animate(1);
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

        PLAYER_STATE_ARRAY[0] = STATE_EXERCISE_ARMS_CENTER;
        PLAYER_STATE_ARRAY[1] = STATE_EXERCISE_ARMS_UP;
        PLAYER_STATE_ARRAY[2] = STATE_EXERCISE_ARMS_CENTER;
        PLAYER_STATE_ARRAY[3] = STATE_EXERCISE_ARMS_WIDE;

        house_get_position_xy(POS_MID_COUCH,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 5;
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_SIDE_VIEW;
        g_hatas = 8;
        lcp_wait_head_reach_target();

        duration = (unsigned short) Random();
        i = 0;
        while (i < ((duration & 0x7f) | 8) &&
               g_trel[0] == ACTION_NONE) {
                lcp_state = PLAYER_STATE_ARRAY[i & 3];
                if (lcp_state == STATE_EXERCISE_ARMS_CENTER)
                        game_tick_and_animate(0);
                else
                        game_tick_and_animate(3);
                i = i + 1;
        }
        lcp_state = STATE_STAND_SIDE_VIEW;
        game_tick_and_animate(0);
}

/* a_chefd: walk to the door, open it, look outside for
   `value` ticks, then optionally close.  value is passed by the do_action
   dispatcher as 40 (see actions.c switch).
   addr: a_chefd() */

void
a_chefd(value)
short   value;
{
        short   result;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        if (lcp_front_door_open == NO)
                a_opcfd(0);
        g_actif = YES;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_walk_to_destination();

        g_selaf[SPRITE_DOG_SIT] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOG_SIT);
        g_sepex[g_seslm[SPRITE_DOG_SIT]] = 294;
        g_sepey[g_seslm[SPRITE_DOG_SIT]] = 151;

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        lcp_walk_to_destination();
        hide_lcp_sprites();
        game_tick_and_animate(value);
        show_lcp_sprites();

        house_get_position_xy(POS_BTM_FRONT_DOOR,
                              &g_wtx, &g_wty);
        g_wtx = g_wtx - 10;
        lcp_walk_to_destination();
        g_selaf[SPRITE_DOG_SIT] = SPRITE_HIDDEN;
        sp_upds();

        result = randomRange(0, 100);
        if (lcp.initiative_threshold < result) {
                g_actif = YES;
                house_get_position_xy(POS_BTM_FRONT_DOOR,
                                      &g_wtx, &g_wty);
                lcp_walk_to_destination();
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
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

        house_get_position_xy(POS_TOP_FILING_CABINET,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        a_watat();

        result = randomRange(0, 100);
        if (lcp.initiative_threshold < result ||
            intro_sequence_active != NO)
                a_opcfc();
}

/* a_cleau: sweep all open doors/cabinets and close them.
   Order matters -- upstairs first (filing cabinet, study door) so the
   dresser/closet close animation doesn't collide with the toilet-door
   sprite pipeline.
   addr: a_cleau() */

void
a_cleau()
{
        short   result;

        if (lcp_filing_cabinet_open != NO) {
                house_get_position_xy(POS_TOP_FILING_CABINET,
                                      &g_wtx, &g_wty);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                a_opcfc();
        }
        if (lcp_study_door_open != NO) {
                house_get_position_xy(POS_TOP_STUDY_DOOR,
                                      &g_wtx, &g_wty);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state            = STATE_STAND_FACING_SCREEN;
                lcp_wait_head_reach_target();
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                od_draw(g_obi07, 178, 23);
                game_tick_and_animate(2);
                od_draw(g_obids,  178, 23);
                sf_sele(SFX_DOOR_CLOSE, 6L);
                game_tick_and_animate(2);
                lcp_study_door_open = NO;
                lcp_facing_direction = FACING_RIGHT;
                lcp_state = STATE_STAND_FACING_SCREEN;
                game_tick_and_animate(0);
        }
        if (lcp_toilet_door_open != NO) {
                house_get_position_xy(POS_MID_TOILET_DOOR,
                                      &g_wtx, &g_wty);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                a_clotd();
        }
        if (lcp_closet_door_open != NO) {
                house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                                      &g_wtx, &g_wty);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                a_clocd();
        }
        if (lcp_dresser_open != NO) {
                house_get_position_xy(POS_MID_DRESSER,
                                      &g_wtx, &g_wty);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                a_opecd(1);
        }
        if (lcp_cabinet_open != NO) {
                house_get_position_xy(POS_BTM_KITCHEN_CABINET,
                                      &g_wtx, &g_wty);
                if ((result = lcp_walk_to_destination()) != 0)
                        return;
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                a_opecc(1);
        }
        if (lcp_front_door_open != NO) {
                walk_to_front_door();
                lcp_facing_direction   = FACING_RIGHT;
                lcp_state              = STATE_STAND_FACING_SCREEN;
                g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
                lcp_wait_head_reach_target();
                a_opcfd(1);
        }
}

/* a_opcbc: 3-sprite dress-change sequence.
   Closet door swings open, then a 3-frame in-closet animation while
   the palette gets swapped to the newly-picked clothing/skin colours,
   then door swings back.  value=0 -> pa_cloc,
   value=1 -> pa_skic.
   addr: a_opcbc() */

void
a_opcbc(value)
short   value;
{
        short   result;
        short   saved_x;
        short   counter;

        house_get_position_xy(POS_MID_DRESSER,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();
        a_opecd(0);
        counter = randomRange(0, 100);
        if (lcp.initiative_threshold < counter)
                a_opecd(1);

        house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_walk_to_destination();
        g_actif = NO;

        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        if (lcp_closet_door_open == NO) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                od_draw(g_obidc, 75, 87);
                game_tick_and_animate(2);
                od_draw(g_obi03, 75, 87);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                od_draw(g_obi04, 75, 87);
                game_tick_and_animate(2);
                lcp_closet_door_open = YES;
        }

        /* Walk into the closet. */
        lcp_facing_direction = FACING_RIGHT;
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 87;

        house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 3;
        g_wtx = g_wtx - 10;
        g_actif = YES;
        lcp_walk_to_destination();
        saved_x = lcp_x;
        g_actif = NO;

        /* Close door behind: wide -> ajar -> lcp-inside. */
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_AJAR);
        g_sepex[g_seslm[SPRITE_CLOSET_AJAR]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_AJAR]] = 87;
        od_draw(g_obi03, 75, 87);
        game_tick_and_animate(1);

        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_LCP_INSIDE] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_LCP_INSIDE);
        hide_lcp_sprites();
        g_sepex[g_seslm[SPRITE_CLOSET_LCP_INSIDE]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_LCP_INSIDE]] = 87;
        od_draw(g_obidc, 75, 87);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(1);

        counter = randomRange(45, 60);
        game_tick_and_animate(counter);
        if (intro_sequence_active == NO) {
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
        show_lcp_sprites();
        g_sepex[g_seslm[SPRITE_CLOSET_AJAR]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_AJAR]] = 87;
        od_draw(g_obi03, 75, 87);
        sf_sele(SFX_DOOR_OPEN, 6L);
        game_tick_and_animate(1);

        g_selaf[SPRITE_CLOSET_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_CLOSET_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 75;
        g_sepey[g_seslm[SPRITE_CLOSET_WIDE_OPEN]] = 87;
        od_draw(g_obi04, 75, 87);
        game_tick_and_animate(1);
        lcp_closet_door_open = YES;

        lcp_x = saved_x;
        house_get_position_xy(POS_MID_BEDROOM_CLOSET,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_walk_to_destination();
        g_actif = NO;

        if (lcp_closet_door_open != NO) {
                g_selaf[SPRITE_CLOSET_WIDE_OPEN] = SPRITE_HIDDEN;
                sp_upds();
                game_tick_and_animate(0);
        }

        counter = randomRange(0, 100);
        if (lcp.initiative_threshold < counter ||
            intro_sequence_active != NO)
                a_clocd();
}

/* a_opcuc: walk to the study door, open it
   if closed (3-frame sprite animation), then walk into the study.
   Chains into lcp_enter_study_and_save; the `value` argument selects
   whether the entry saves (value != 0 -> do_save=YES) or just plays
   the animation (value == 0 -> do_save=NO).  Both entries play the
   door SFX.
   addr: a_opcuc() */

void
a_opcuc(value)
short   value;
{
        short   result;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        result = lcp_walk_to_destination();
        if (result != 0)
                return;

        g_hamod         = HEAD_ANIM_DISABLED;
        lcp_facing_direction   = FACING_RIGHT;
        lcp_state              = STATE_STAND_FACING_SCREEN;
        g_hatas = HEAD_ANIM_HORIZONTAL_RANGE;
        lcp_wait_head_reach_target();

        if (lcp_study_door_open == NO) {
                lcp_facing_direction = FACING_LEFT;
                lcp_state = STATE_BEND_AND_REACH;
                game_tick_and_animate(2);
                od_draw(g_obids,  178, 23);
                game_tick_and_animate(2);
                od_draw(g_obi07,  178, 23);
                sf_sele(SFX_DOOR_OPEN, 6L);
                game_tick_and_animate(2);
                od_draw(g_obi08,  178, 23);
                game_tick_and_animate(2);
                lcp_study_door_open = YES;
        }

        /* Walk into the study, ducking behind the wide-open door. */
        lcp_facing_direction = FACING_RIGHT;
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;

        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_wty = g_wty - 3;
        g_wtx = g_wtx - 10;
        g_actif = YES;
        lcp_walk_to_destination();
        g_actif = NO;

        /* Swap wide-open sprite for ajar and hide the resident. */
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] =  23;
        od_draw(g_obi07, 178, 23);
        hide_lcp_sprites();
        game_tick_and_animate(1);
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();

        /* Continue into the study; value != 0 -> save HYBER. */
        if (value == 0)
                lcp_enter_study_and_save(NO,  YES);
        else
                lcp_enter_study_and_save(YES, YES);
}
