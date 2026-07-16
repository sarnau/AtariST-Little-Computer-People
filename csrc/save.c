/*
 * save.c -- HYBER save file I/O and the study-door save flow.
 *
 * The Atari ST version persists 128 bytes of PLAYER state to a
 * single file named "hyber" in the current directory.  Load happens
 * once at startup; save happens whenever the resident walks into the
 * study, closes the door, and does the "packing" animation.
 *
 *   create_file()          -- ensures the target exists via GEMDOS Fcreate
 *   fr_read()            -- retrying GEMDOS Fread with error alert
 *   lcp_save()             -- writes N bytes to a named file (128 in practice)
 *   lc_load()             -- reads 128 bytes and unpacks
 *                            door_states_and_flags into per-door globals
 *   lcp_enter_study_and_save() -- packs runtime state back into the PLAYER
 *                            struct, calls lcp_save, and runs the study
 *                            enter/exit animation.
 *
 * addr: create_file(), fr_read(), lcp_save(), lc_load(),
 *       lcp_enter_study_and_save()
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
extern short    lcp_water_level;
extern BOOL16   g_actif;
extern short    g_wtx;
extern short    g_wty;
extern void     game_tick_and_animate();
extern short    lcp_front_door_open;
extern short    lcp_study_door_open;
extern short    lcp_closet_door_open;
extern short    lcp_cabinet_open;
extern short    lcp_dresser_open;
extern short    lcp_toilet_door_open;
extern short    lcp_filing_cabinet_open;
extern short    lcp_dog_bowl_status;
extern short    lcp_food_count;
extern short    lcp_record_playing;
extern short    lcp_tv_on;
extern short    g_obids;
extern short    g_obi07;
extern short    g_obi08;
extern void     house_get_position_xy();
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_selaf[];
extern short    g_seslm[];
extern short    randomRange();                  /* random.c */
extern void     lcp_update_palette_colors();    /* render.c  */
#include <osbind.h>
#include <stdio.h>

/* Externals resolved elsewhere. */
extern void     sp_sprs();
extern void     od_draw();
extern void     sf_sele();
extern void     sp_upds();
extern void     lcp_update_palette_colors();
extern void     show_lcp_sprites();
extern short    lcp_walk_to_destination();
extern short    randomRange();
extern void     error_unable_to_write();

/* file_open: retrying GEMDOS Fopen.  rwmode: 0=read, 1=write, 2=both.
   Same retry-then-alert pattern as fr_read/lcp_save -- three tries
   with a 1-second sleep, then loop with a Retry alert.
   addr: file_open() */

short
file_open(filename, rwmode)
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

/* create_file: idempotent GEMDOS Fcreate.  Uses access() to see if the
   file already exists; if not, keep retrying Fcreate until it succeeds.
   The original quietly Fcloses the temporary handle it opened -- we
   preserve that so the file is closed even in the success path.

   addr: create_file() */

void
create_file(filename)
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
                error_unable_to_write();
        }
        _gemdos(GEMDOS_Fclose, iVar1, 0L, 0L);
}

/* fr_read: GEMDOS Fread with retry-then-alert error handling.  After
   three failed attempts, throws a Retry alert and loops.

   addr: fr_read() */

void
fr_read(fileHandle, count, buffer)
short   fileHandle;
long    count;
void *  buffer;
{
        short   err;
        short   retry;

        retry = 0;
        for (;;) {
                /* GEMDOS Fread expects: func(word), handle(word),
                   count(long), buffer(long).  Pass fileHandle as its
                   NATURAL SHORT type -- the `(long)` cast that used
                   to be here pushed 4 bytes where TOS wanted 2, so
                   the trap read handle = 0 (the high word of the long)
                   and every file was silently being read from stdin
                   with a bogus (huge) count. */
                err = _gemdos(GEMDOS_Fread, fileHandle, count, (long) buffer);
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

/* file_load: open + read header + read payload + close.  The 1985
   .lcp/.pex files carry a 4-byte header of two shorts -- the first
   short is discarded (temp), the second is the payload byte count.
   Ghidra:
       fileHandle = file_open(filename, 0);
       file_read(fileHandle, 2, &temp);
       file_read(fileHandle, 2, &size);
       file_read(fileHandle, size, buffer);
       _gemdos(GEMDOS_Fclose, fileHandle);
   addr: file_load() */

void
file_load(filename, buffer)
char *  filename;
void *  buffer;
{
        short   fileHandle;
        short   size;
        short   temp;

        fileHandle = file_open(filename, 0);
        fr_read(fileHandle, 2L, &temp);
        fr_read(fileHandle, 2L, &size);
        fr_read(fileHandle, (long) size, buffer);
        _gemdos(GEMDOS_Fclose, fileHandle, 0L, 0L);
}

/* lcp_save: create + open + write + close a file, retrying on every
   failure via error_unable_to_write() (which pops a Retry alert).
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

        create_file(filename);

        for (;;) {
                filehandle = _gemdos(GEMDOS_Fopen, (long) filename, 1L, 0L);
                if (filehandle >= 0)
                        break;
                error_unable_to_write();
        }

        for (;;) {
                lVar1 = _gemdos(GEMDOS_Fwrite, filehandle,
                                (long) size, (long) addr);
                if (lVar1 == (long) size)
                        break;
                error_unable_to_write();
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
        short   fileHandle;

        fileHandle = _gemdos(GEMDOS_Fopen, (long) "hyber", 0L, 0L);
        if (fileHandle < 0)
                return 0;

        fr_read(fileHandle, 0x80L, &lcp);
        _gemdos(GEMDOS_Fclose, fileHandle, 0L, 0L);

        lcp_water_level         = lcp.water_level;
        lcp_front_door_open     = lcp.door_states_and_flags & DSF_FRONT_DOOR;
        lcp_dresser_open        = (lcp.door_states_and_flags & DSF_DRESSER)          >> 4;
        lcp_cabinet_open        = (lcp.door_states_and_flags & DSF_KITCHEN_CABINET)  >> 3;
        lcp_closet_door_open    = (lcp.door_states_and_flags & DSF_CLOSET_DOOR)      >> 2;
        lcp_study_door_open     = (lcp.door_states_and_flags & DSF_STUDY_DOOR)       >> 1;
        lcp_toilet_door_open    = (lcp.door_states_and_flags & DSF_TOILET_DOOR)      >> 5;
        lcp_filing_cabinet_open = (lcp.door_states_and_flags & DSF_FILING_CABINET)   >> 6;
        lcp_dog_bowl_status     = (lcp.door_states_and_flags & DSF_DOG_BOWL_MASK)    >> 7;
        lcp_food_count          = lcp.food_supply;
        lcp_record_playing      = lcp.record_playing;
        lcp_tv_on               = lcp.tv_on;

        lcp_update_palette_colors();
        return 1;
}

/* lcp_enter_study_and_save: three-phase animation:
     1. Study door closes behind the resident (SPRITE_DOOR_STUDY_1).
     2. Optionally write PLAYER -> HYBER (do_save flag).
     3. Study door swings back open (SPRITE_DOOR_STUDY_AJAR ->
        SPRITE_DOOR_STUDY_WIDE_OPEN), resident walks back to the door,
        then the door closes.

   The bit-field for door_states_and_flags is repacked from the eight
   per-door runtime globals; the food-count nibble (bits 9..11) is
   preserved via the FE00 mask so the delivery event handler's 3-bit
   counter survives the save.

   addr: lcp_enter_study_and_save() */

void
lcp_enter_study_and_save(do_save, p_dosnd)
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

        game_tick_and_animate(1);
        counter = randomRange(15, 30);
        game_tick_and_animate(counter);

        /* Phase 2: repack door state and write HYBER. */
        if (do_save != NO) {
                lcp.water_level = lcp_water_level;
                lcp.door_states_and_flags =
                        lcp_front_door_open |
                        (lcp_dog_bowl_status     << 7) |
                        (lcp_filing_cabinet_open << 6) |
                        (lcp_toilet_door_open    << 5) |
                        (lcp_dresser_open        << 4) |
                        (lcp_cabinet_open        << 3) |
                        (lcp_closet_door_open    << 2) |
                        (lcp_study_door_open     << 1) |
                        (lcp.door_states_and_flags & DSF_PRESERVE_UPPER_MASK);
                lcp.record_playing = lcp_record_playing;
                lcp.tv_on          = lcp_tv_on;
                lcp.food_supply    = lcp_food_count;
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
        game_tick_and_animate(1);

        /* Phase 3b: door wide open, resident visible. */
        g_selaf[SPRITE_DOOR_STUDY_AJAR] = SPRITE_HIDDEN;
        sp_upds();
        g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] = SPRITE_IN_FRONT;
        sp_sprs(SPRITE_DOOR_STUDY_WIDE_OPEN);
        g_sepex[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] = 178;
        g_sepey[g_seslm[SPRITE_DOOR_STUDY_WIDE_OPEN]] =  23;
        od_draw(g_obi08, 178, 23);
        show_lcp_sprites();
        game_tick_and_animate(1);

        /* Phase 4: walk resident back to the study door. */
        lcp_x = saved_x;
        house_get_position_xy(POS_TOP_STUDY_DOOR,
                              &g_wtx, &g_wty);
        g_actif = YES;
        lcp_walk_to_destination();
        g_actif = NO;

        /* Phase 5: close door, clear the "study door open" flag. */
        if (lcp_study_door_open != NO) {
                g_selaf[SPRITE_DOOR_STUDY_WIDE_OPEN] =
                        SPRITE_HIDDEN;
                sp_upds();
                game_tick_and_animate(0);
        }
        od_draw(g_obi07, 178, 23);
        game_tick_and_animate(2);
        od_draw(g_obids,  178, 23);
        sf_sele(SFX_DOOR_CLOSE, 6L);
        game_tick_and_animate(2);
        lcp_study_door_open = NO;
}
