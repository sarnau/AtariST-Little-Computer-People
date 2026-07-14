/*
 * structs.h -- game struct layouts (Ghidra-verified against LCP.PRG).
 *
 * Field order and sizes must match the original binary layout; the
 * 128-byte HYBER save file loads directly into the LCP struct via
 * memcpy(). Keep offsets in sync with lcp/structs.py.
 *
 * addr: LCP struct at Ghidra symbol "lcp"; HYBER file layout.
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#include "types.h"

/* PLAYER (LCP) -- 128-byte persistent character state.
   Layout verified via Ghidra struct editor and HYBER save file dumps. */
typedef struct {
        /* Appearance                                       0x00 */
        short   clothing_color;
        short   skin_color;

        /* Daily schedule                                   0x04 */
        short   bedtime_hour;
        short   wake_hour;
        short   lunch_hour;
        short   dinner_hour;

        /* Personality                                      0x0C */
        short   personality_type;
        short   activity_level;

        /* Reserved (24 bytes, no code references)          0x10 */
        char    _reserved_10[24];

        /* Happiness                                        0x28 */
        short   happiness;
        short   happiness_initial_countdown;
        short   happiness_duration_happy;
        short   happiness_duration_content;
        short   happiness_duration_active;
        short   happiness_direction;

        /* Sickness                                         0x34 */
        short   sickness_level;
        short   sickness_countdown;
        short   sickness_direction;

        /* Sleep                                            0x3A */
        short   is_sleeping;

        /* Initiative                                       0x3C */
        short   initiative_threshold;

        /* Thirst                                           0x3E */
        short   thirst_level;
        short   thirst_timer_max;
        short   thirst_timer;

        /* Hunger                                           0x44 */
        short   hunger_level;
        short   hunger_timer_max;
        short   hunger_timer;

        /* Bathroom                                         0x4A */
        short   bathroom_need;
        short   bathroom_timer_max;
        short   bathroom_timer;

        /* Reserved                                         0x50 */
        short   _reserved_50;

        /* Items / state                                    0x52 */
        short   food_supply;
        short   record_playing;
        short   tv_on;
        short   door_states_and_flags;

        /* Character ID                                     0x5A */
        short   character_sprite_id;
        short   water_level;

        /* Names                                            0x5E */
        char    owner_name[24];
        char    character_name[10];
} PLAYER;

/* PSG_ENVELOPE -- ADSR envelope state for one YM2149 PSG channel.
   Field order matches the 8-byte layout of the ADSR parameter block
   in Activision Music Studio 2.0's .SNG / .ORG files (post-signature
   body offset 0x00..0x1A3), so psg_copy_envelope_params can memcpy
   directly from the file bytes into the runtime struct.  See
   sound.c:song_play for the full Music Studio file format. */
typedef struct {
        char    phase;                  /* ENV_ATTACK..ENV_FADEOUT     */
        char    phase_timer;            /* ticks until next phase step */
        unsigned char attack_start_vol; /* starting volume (0..15)     */
        unsigned char attack_duration;  /* attack length in ticks      */
        unsigned char decay_duration;   /* decay length                */
        unsigned char sustain_level;    /* sustain volume level        */
        unsigned char release_duration; /* release length              */
        unsigned char current_volume;   /* live PSG volume for channel */
        unsigned char max_volume;       /* velocity-derived ceiling    */
} PSG_ENVELOPE;

/* WORD_TO_ACTION -- one entry in the parser's command-matching table.
   `table[10]` is a per-position bitmask: for each of the 10 position
   slots, all bits that must be present in the accumulated
   g_ewb[] before this entry matches.  A sentinel entry
   with `table[0] == 0xff` terminates the table.  `priority_offset`
   nudges the action's queue priority up or down; `action` is the
   ACTION_ID to fire (stored as char to save 1 byte per row -- the
   1985 code cared about ROM footprint). */
typedef struct {
        unsigned char   table[10];
        short           priority_offset;
        char            action;
} WORD_TO_ACTION;

/* MFDB -- VDI memory form definition block.  Passed as source or
   destination descriptor to vro_cpyfm and related raster ops.
   Layout matches the GEM VDI ABI exactly (Alcyon's <mfdb.h>). */
typedef struct {
        void *  fd_addr;                /* pointer to bitmap data      */
        short   fd_w;                   /* width in pixels             */
        short   fd_h;                   /* height in pixels            */
        short   fd_wdwidth;             /* width in words (fd_w / 16)  */
        short   fd_stand;               /* 0=device, 1=standard format */
        short   fd_nplanes;             /* number of bit planes        */
        short   fd_r1;                  /* reserved */
        short   fd_r2;
        short   fd_r3;
} MFDB;

/* RECT16 -- 4-corner rectangle used for VDI polylines.
   Laid out so a `short *` can be passed to v_pline() and it walks the
   four coordinates in x1,y1,x2,y2 order (matching the GEM VDI ABI). */
typedef struct {
        short   x1;
        short   y1;
        short   x2;
        short   y2;
} RECT16;

#endif  /* STRUCTS_H */
