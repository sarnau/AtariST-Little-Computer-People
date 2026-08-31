/*
 * save.c -- HYBER save file I/O and the study-door save flow.
 * addr: crFile(), fr_read(), lcp_save(), lc_load(), lcp_std()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include <stdio.h>
#include "alerts.h"
#include "globals.h"
#include "movement.h"
#include "random.h"
#include "render.h"
#include "renderx.h"
#include "save.h"
#include "sound.h"
#include "sprglobs.h"
#include "sprites.h"
#include "tick.h"
#include "walk.h"

/* rwmode: 0=read, 1=write, 2=both.  Three tries with a 1s sleep, then
   Retry alert loop.
   addr: fOpen() */
short
fOpen(filename, rwmode)
char *  filename;
short   rwmode;
{
        short   fhandle;
        short   retry;

        retry = 0;
        for (;;) {
                /* ROM pushes a trailing 0L after the mode word. */
                fhandle = gemdos(0x3D, filename, rwmode, 0L);
                if (fhandle >= 0)
                        return fhandle;
                retry = retry + 1;
                if (retry < 3)
                        evnt_timer(1000, 0);
                else
                        form_alert(0,
                                "[1][Bad file open.|Try re-booting.][RETRY]");
        }
}

/* addr: crFile() */
void
crFile(filename)
char *  filename;
{
        short   rval;
        short   iVar1;

        rval = access(filename, 4);
        if (rval == 0)
                return;

        for (;;) {
                /* ROM: two trailing 0L args on create and close. */
                iVar1 = gemdos(0x3C, filename, 0L, 0L);
                if (iVar1 >= 0)
                        break;
                er_write();
        }
        gemdos(0x3E, iVar1, 0L, 0L);
}

/* addr: fr_read() */
void
fr_read(fhnd, count, buffer)
short   fhnd;
long    count;
void *  buffer;
{
        short   err;
        short   retry;

        retry = 0;
        for (;;) {
                /* Fread expects handle as word; a (long) cast here pushes
                   4 bytes where TOS wants 2 and silently reads from
                   handle 0.  Keep fhnd as short. */
                err = Fread(fhnd, count, buffer);
                if (err >= 0)
                        return;
                retry = retry + 1;
                if (retry < 3)
                        evnt_timer(1000, 0);
                else
                        form_alert(0,
                                "[1][Bad file read.|Try re-booting.][RETRY]");
        }
}

/* 4-byte header: discarded temp short, then payload size short.
   addr: fLoad() */
void
fLoad(filename, buffer)
char *  filename;
void *  buffer;
{
        short   fhnd;
        short   size;
        short   temp;

        fhnd = fOpen(filename, 0);
        fr_read(fhnd, 2L, &temp);
        fr_read(fhnd, 2L, &size);
        fr_read(fhnd, (long) size, buffer);
        gemdos(0x3E, fhnd, 0L, 0L);     /* ROM: two trailing 0L args */
}

/* addr: lcp_save() */
void
lcp_save(filename, size, addr)
char *  filename;
short   size;
void *  addr;
{
        short   filehandle;
        long    lVar1;

        crFile(filename);

        for (;;) {
                /* ROM pushes a trailing 0L after the long mode. */
                filehandle = gemdos(0x3D, filename, 1L, 0L);
                if (filehandle >= 0)
                        break;
                er_write();
        }

        for (;;) {
                lVar1 = gemdos(0x40, filehandle, (long) size, addr);
                /* ROM evaluates the size cast first. */
                if ((long) size == lVar1)
                        break;
                er_write();
        }

        gemdos(0x3E, filehandle, 0L, 0L);       /* two trailing 0L args */
}

/* addr: lc_load() */
short
lc_load()
{
        short   fhnd;

        /* ROM: mode is 0L and a second trailing 0L follows. */
        fhnd = gemdos(0x3D, "hyber", 0L, 0L);
        if (fhnd < 0)
                return 0;

        fr_read(fhnd, 0x80L, &lcp);
        gemdos(0x3E, fhnd, 0L, 0L);     /* two trailing 0L args */

        lcp_watr         = lcp.water_level;
        lcp_frdO     = lcp.door_states_and_flags & DSF_FRONT_DOOR;
        lcp_drsO        = (lcp.door_states_and_flags & DSF_DRESSER)          >> 4;
        lcp_cabO        = (lcp.door_states_and_flags & DSF_KITCHEN_CABINET)  >> 3;
        lcp_clsO    = (lcp.door_states_and_flags & DSF_CLOSET_DOOR)      >> 2;
        studyDrO     = (lcp.door_states_and_flags & DSF_STUDY_DOOR)       >> 1;
        lcp_toiO    = (lcp.door_states_and_flags & DSF_TOILET_DOOR)      >> 5;
        lcp_flcO = (lcp.door_states_and_flags & DSF_FILING_CABINET)   >> 6;
        lcp_bwlS     = (lcp.door_states_and_flags & DSF_DOG_BOWL_MASK)    >> 7;
        lcp_food          = lcp.food_supply;
        lcp_recP      = lcp.record_playing;
        lcp_tv               = lcp.tv_on;

        lcp_upal();
        return 1;
}

/* Study-door save flow: close door, optionally write HYBER, reopen,
   walk resident back to door, close.  Food-count nibble (bits 9..11)
   is preserved via the FE00 mask so the 3-bit delivery counter survives.
   addr: lcp_std() */
void
lcp_std(do_save, p_dosnd)
BOOL16  do_save;
BOOL16  p_dosnd;
{
        short   saved_x;
        short   counter;

        saved_x = lcp_x;

        /* Phase 1: door closes (sprite in front of the resident). */
        g_selaf[SPRITE_DOOR_STUDY_1] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_1);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_1]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_1]] =  23;
        od_draw(od_stcl, 178, 23);

        if (p_dosnd != NO)
                sf_sele(SFX_DOOR_CLOSE, 6L);

        gameTick(1);
        counter = rndRng(15, 30);
        gameTick(counter);

        /* Phase 2: repack door state and write HYBER. */
        if (do_save != NO) {
                lcp.water_level = lcp_watr;
                lcp.door_states_and_flags =
                        lcp_frdO |
                        (lcp_bwlS     << 7) |
                        (lcp_flcO << 6) |
                        (lcp_toiO    << 5) |
                        (lcp_drsO        << 4) |
                        (lcp_cabO        << 3) |
                        (lcp_clsO    << 2) |
                        (studyDrO     << 1) |
                        (lcp.door_states_and_flags & DSF_PRESERVE_UPPER_MASK);
                lcp.record_playing = lcp_recP;
                lcp.tv_on          = lcp_tv;
                lcp.food_supply    = lcp_food;
                lcp_save("hyber", 0x80, &lcp);
        }

        /* Phase 3a: door swings ajar. */
        g_selaf[SPRITE_DOOR_STUDY_1] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_AJAR);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_AJAR]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_AJAR]] =  23;
        od_draw(od_sto1, 178, 23);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        /* Phase 3b: door wide open, resident visible. */
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;
        od_draw(od_sto2, 178, 23);
        showLcp();
        gameTick(1);

        /* Phase 4: walk resident back to the study door. */
        lcp_x = saved_x;
        hs_posXY(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_wkD();
        g_actif = NO;

        /* Phase 5: close door, clear the "study door open" flag. */
        if (studyDrO != NO) {
                g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] =
                        SPRITE_HIDDEN;
                sp_upds();
                gameTick(0);
        }
        od_draw(od_sto1, 178, 23);
        gameTick(2);
        od_draw(od_stcl, 178, 23);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        studyDrO = NO;
}
