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
   14-byte runtime layout matching Ghidra's psg_envelope struct
   (`muls.w #0xe` at 0x115c0 confirms sizeof = 0xe = 14).  The 8-byte
   on-disk ADSR parameter block from Activision Music Studio 2.0's
   .SNG / .ORG files maps onto offsets 1..8 (attack_start_vol
   through release_duration), so psg_cpE can memcpy directly into
   the runtime struct from an 8-byte source buffer without touching
   the phase / ramp_direction / phase_timer / current_volume /
   max_volume fields that live outside the on-disk window.

   Field offsets are hand-controlled with explicit byte padding
   because Alcyon C 4.14 doesn't guarantee any specific alignment
   for `short`s within structs (usually 2-byte, but the ROM's
   0xe total confirms no padding between offset 9 and offset 10). */
typedef struct {
        char            phase;                  /* off 0  ENV_ATTACK..    */
        unsigned char   attack_start_vol;       /* off 1  volume 0..15    */
        unsigned char   attack_duration;        /* off 2  attack ticks    */
        unsigned char   attack_target_vol;      /* off 3  peak volume     */
        unsigned char   decay_duration;         /* off 4                  */
        unsigned char   decay_target_vol;       /* off 5                  */
        unsigned char   sustain_duration;       /* off 6                  */
        unsigned char   sustain_target_vol;     /* off 7                  */
        unsigned char   release_duration;       /* off 8                  */
        char            ramp_direction;         /* off 9  +1 or -1        */
        short           phase_timer;            /* off 10 ticks until step*/
        unsigned char   current_volume;         /* off 12 live PSG volume */
        unsigned char   max_volume;             /* off 13 vel-derived cap */
} PSG_ENVELOPE;

/* WORD_TO_ACTION -- one entry in the parser's command-matching table.
   12-byte layout confirmed against Ghidra (auto-generated labels at
   enteredword_to_action[0] show action @ +10, priorityOffset @ +11,
   struct size 12).  `table[10]` is a per-position bitmask: for each
   of the 10 position slots, all bits that must be present in the
   accumulated g_ewb[] before this entry matches.  A sentinel entry
   with `table[0] == 0xff` terminates the table.  `action` is the
   ACTION_ID to fire; `priority_offset` nudges the action's queue
   priority up or down.  Both are single bytes. */
typedef struct {
        unsigned char   table[10];
        char            action;
        char            priority_offset;
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

/* DTA -- GEMDOS Disk Transfer Address, the record Fsfirst / Fsnext
   write to and Fgetdta returns.  Matches the standard TOS/Alcyon
   layout (see ~/hatari-c/TOOLS/INCLUDE/ostruct.h `_DTA`).  Total 44
   bytes.  Callers here read `d_length` (song file loader) and
   `d_fname` (song enumeration). */
typedef struct {
        char    d_reserved[21];         /* reserved for GEMDOS */
        char    d_attrib;               /* file attributes */
        short   d_time;                 /* packed time */
        short   d_date;                 /* packed date */
        long    d_length;               /* file size */
        char    d_fname[14];            /* filename (8.3 uppercase) */
} DTA;

#endif  /* STRUCTS_H */
