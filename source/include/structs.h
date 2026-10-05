/*
 * structs.h -- game struct layouts.
 *
 * Field order and sizes must match the original binary layout; the
 * 128-byte HYBER save file loads directly into the PLAYER struct.
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#include "types.h"
#ifdef HOST
#include "hostgem.h"
#else
#include <vdibind.h>    /* MFDB */
#include <ostruct.h>    /* _DTA */
#endif

/* PLAYER (LCP) -- 128-byte persistent character state; also the
   layout of the HYBER save file. */
typedef struct {
        /* Appearance                                       0x00 */
        short   clothingColor;
        short   skinColor;

        /* Daily schedule                                   0x04 */
        short   bedtimeHour;
        short   wakeHour;
        short   lunchHour;
        short   dinnerHour;

        /* Personality                                      0x0C */
        short   personalityType;
        short   activityLevel;

        /* Reserved (24 bytes, no code references)          0x10 */
        char    reserved10[24];

        /* Happiness                                        0x28 */
        short   happiness;
        /* Length of a spell in each mood, indexed by MOOD_*; the active
           one is reloaded from here on every change. */
        short   moodDuration[3];
        short   happinessDurationActive;
        short   happinessDirection;

        /* Sickness                                         0x34 */
        short   sicknessLevel;
        short   sicknessCountdown;
        short   sicknessDirection;

        /* Sleep                                            0x3A */
        BOOL16  isSleeping;

        /* Initiative                                       0x3C */
        short   initiativeThreshold;

        /* Thirst                                           0x3E */
        short   thirstLevel;
        short   thirstTimerMax;
        short   thirstTimer;

        /* Hunger                                           0x44 */
        short   hungerLevel;
        short   hungerTimerMax;
        short   hungerTimer;

        /* Bathroom                                         0x4A */
        BOOL16  bathroomNeed;
        short   bathroomTimerMax;
        short   bathroomTimer;

        /* Reserved                                         0x50 */
        short   reserved50;

        /* Items / state                                    0x52 */
        short   recordCount;
        BOOL16  recordPlaying;
        BOOL16  tvOn;
        short   doorStatesAndFlags;

        /* Character ID                                     0x5A */
        short   characterSpriteId;
        short   waterLevel;

        /* Names                                            0x5E */
        char    ownerName[24];
        char    characterName[10];
} PLAYER;

/* PSG_ENVELOPE -- ADSR envelope state for one YM2149 PSG channel.
   14-byte runtime layout (the code indexes the array with a stride
   of 14).  The 8-byte
   on-disk ADSR parameter block from Activision Music Studio 2.0's
   .SNG / .ORG files maps onto offsets 1..8 (attackStartVol
   through releaseDuration), so copyEnvelope can memcpy directly into
   the runtime struct from an 8-byte source buffer without touching
   the phase / rampDirection / phaseTimer / currentVolume /
   maxVolume fields that live outside the on-disk window.

   Field offsets are hand-controlled with explicit byte padding
   because Alcyon C 4.14 doesn't guarantee any specific alignment
   for `short`s within structs (usually 2-byte; the 14-byte total
   means there is no padding between offset 9 and offset 10). */
typedef struct {
        char            phase;                  /* off 0  ENV_ATTACK..    */
        unsigned char   attackStartVol;         /* off 1  volume 0..15    */
        unsigned char   attackDuration;         /* off 2  attack ticks    */
        unsigned char   attackTargetVol;        /* off 3  peak volume     */
        unsigned char   decayDuration;          /* off 4                  */
        unsigned char   decayTargetVol;         /* off 5                  */
        unsigned char   sustainDuration;        /* off 6                  */
        unsigned char   sustainTargetVol;       /* off 7                  */
        unsigned char   releaseDuration;        /* off 8                  */
        unsigned char   maxVolume;              /* off 9  vel-derived cap */
        short           phaseTimer;             /* off 10 ticks until step*/
        unsigned char   currentVolume;          /* off 12 live PSG volume */
        char            rampDirection;          /* off 13 +1 or -1        */
} PSG_ENVELOPE;

/* WORD_TO_ACTION -- one entry in the parser's command-matching table.
   12-byte layout (matchCommand walks the rows with a stride of 12):
   `table[10]` signed bitmask bytes, the ACTION_ID byte at +10, then
   the priority offset byte at +11.  A sentinel entry with
   `table[0] == 0xff` terminates the table. */
typedef struct {
        char            table[10];
        char            action;         /* +10 */
        char            priorityOffset; /* +11 */
} WORD_TO_ACTION;

/* MFDB is defined in <vdibind.h> (included above) -- do not redeclare. */

/* _DTA is defined in <ostruct.h> (included above) -- do not redeclare.
   Callers use _DTA * directly (44 bytes). */

#endif  /* STRUCTS_H */
