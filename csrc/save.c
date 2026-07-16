/*
 * save.c -- HYBER save file I/O and the study-door save flow.
 *
 * The Atari ST version persists 128 bytes of PLAYER state to a
 * single file named "hyber" in the current directory.  Load happens
 * once at startup; save happens whenever the resident walks into the
 * study, closes the door, and does the "packing" animation.
 *
 *   crFile()          -- ensures the target exists via GEMDOS Fcreate
 *   fr_read()            -- retrying GEMDOS Fread with error alert
 *   lcp_save()             -- writes N bytes to a named file (128 in practice)
 *   lc_load()             -- reads 128 bytes and unpacks
 *                            door_states_and_flags into per-door globals
 *   lcp_std() -- packs runtime state back into the PLAYER
 *                            struct, calls lcp_save, and runs the study
 *                            enter/exit animation.
 *
 * addr: crFile(), fr_read(), lcp_save(), lc_load(),
 *       lcp_std()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern short    lcp_x;
extern short    lcp_watr;
extern BOOL16   g_actif;
extern short    g_wtx;
extern short    g_wty;
extern void     gameTick();
extern short    lcp_frdO;
extern short    studyDrO;
extern short    lcp_clsO;
extern short    lcp_cabO;
extern short    lcp_drsO;
extern short    lcp_toiO;
extern short    lcp_flcO;
extern short    lcp_bwlS;
extern short    lcp_food;
extern short    lcp_recP;
extern short    lcp_tv;
extern short    g_obids;
extern short    g_obi07;
extern short    g_obi08;
extern void     hs_posXY();
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    rndRng();                  /* random.c */
extern void     lcp_upal();    /* render.c  */
#include <osbind.h>
#include <stdio.h>

/* Externals resolved elsewhere. */
extern void     sp_sprs();
extern void     od_draw();
extern void     sf_sele();
extern void     sp_upds();
extern void     lcp_upal();
extern void     showLcp();
extern short    lcp_wkD();
extern short    rndRng();
extern void     er_write();

/* fOpen: retrying GEMDOS Fopen.  rwmode: 0=read, 1=write, 2=both.
   Same retry-then-alert pattern as fr_read/lcp_save -- three tries
   with a 1-second sleep, then loop with a Retry alert.
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
                fhandle = _gemdos(GEMDOS_Fopen, (long) filename, rwmode, 0L);
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

/* crFile: idempotent GEMDOS Fcreate.  Uses access() to see if the
   file already exists; if not, keep retrying Fcreate until it succeeds.
   The original quietly Fcloses the temporary handle it opened -- we
   preserve that so the file is closed even in the success path.

   addr: crFile() */

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
                iVar1 = _gemdos(GEMDOS_Fcreate, (long) filename, 0L, 0L);
                if (iVar1 >= 0)
                        break;
                er_write();
        }
        _gemdos(GEMDOS_Fclose, iVar1, 0L, 0L);
}

/* fr_read: GEMDOS Fread with retry-then-alert error handling.  After
   three failed attempts, throws a Retry alert and loops.

   addr: fr_read() */

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
                /* GEMDOS Fread expects: func(word), handle(word),
                   count(long), buffer(long).  Pass fhnd as its
                   NATURAL SHORT type -- the `(long)` cast that used
                   to be here pushed 4 bytes where TOS wanted 2, so
                   the trap read handle = 0 (the high word of the long)
                   and every file was silently being read from stdin
                   with a bogus (huge) count. */
                err = _gemdos(GEMDOS_Fread, fhnd, count, (long) buffer);
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

/* fLoad: open + read header + read payload + close.  The 1985
   .lcp/.pex files carry a 4-byte header of two shorts -- the first
   short is discarded (temp), the second is the payload byte count.
   Ghidra:
       fhnd = fOpen(filename, 0);
       file_read(fhnd, 2, &temp);
       file_read(fhnd, 2, &size);
       file_read(fhnd, size, buffer);
       _gemdos(GEMDOS_Fclose, fhnd);
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
        _gemdos(GEMDOS_Fclose, fhnd, 0L, 0L);
}

/* lcp_save: create + open + write + close a file, retrying on every
   failure via er_write() (which pops a Retry alert).
   Original signature took (filename, size, addr) with size as short --
   preserved verbatim.

   addr: lcp_save() */

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
                filehandle = _gemdos(GEMDOS_Fopen, (long) filename, 1L, 0L);
                if (filehandle >= 0)
                        break;
                er_write();
        }

        for (;;) {
                lVar1 = _gemdos(GEMDOS_Fwrite, filehandle,
                                (long) size, (long) addr);
                if (lVar1 == (long) size)
                        break;
                er_write();
        }

        _gemdos(GEMDOS_Fclose, filehandle, 0L, 0L);
}

/* lc_load: read 128 bytes from "hyber" into the PLAYER struct, unpack
   the door bitfield into per-door runtime globals, and repaint the
   palette (which may depend on lcp.sickness_level).  Returns 1 on
   success, 0 if no save file.

   addr: lc_load() */

short
lc_load()
{
        short   fhnd;

        fhnd = _gemdos(GEMDOS_Fopen, (long) "hyber", 0L, 0L);
        if (fhnd < 0)
                return 0;

        fr_read(fhnd, 0x80L, &lcp);
        _gemdos(GEMDOS_Fclose, fhnd, 0L, 0L);

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

/* lcp_std: three-phase animation:
     1. Study door closes behind the resident (SPRITE_DOOR_STUDY_1).
     2. Optionally write PLAYER -> HYBER (do_save flag).
     3. Study door swings back open (SPRITE_DOOR_STUDY_AJAR ->
        SPRITE_DOOR_STUDY_WIDE_OPEN), resident walks back to the door,
        then the door closes.

   The bit-field for door_states_and_flags is repacked from the eight
   per-door runtime globals; the food-count nibble (bits 9..11) is
   preserved via the FE00 mask so the delivery event handler's 3-bit
   counter survives the save.

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
        od_draw(g_obids, 178, 23);

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
        od_draw(g_obi07, 178, 23);
        sf_sele(SFX_DOOR_OPEN, 6L);
        gameTick(1);

        /* Phase 3b: door wide open, resident visible. */
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;
        od_draw(g_obi08, 178, 23);
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
        od_draw(g_obi07, 178, 23);
        gameTick(2);
        od_draw(g_obids,  178, 23);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        gameTick(2);
        studyDrO = NO;
}
