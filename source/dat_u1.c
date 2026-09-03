/*
 * dat_u1.c -- the initialized globals that belong to the stx_u1
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u1.
 */

/*
 * dat_u1.c -- the initialized globals that belong to the stx_u1
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u1.
 */

/*
 * dat_u1.c -- the initialized globals that belong to the stx_u1
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u1.
 */




/* main_pal[16]: Atari ST 12-bit RGB palette (4 bits per channel).
   Entries 0..15 map to the 16 screen colours in low-res mode.  Values
   dumped from the 1985 data segment at Ghidra 0x29B44 (via
   ghidra_scripts/DumpPalette.java).  aes_init loads this via
   Setpalette(main_pal) at boot -- there is no
   later runtime palette rewrite from this table; slot 0 is the
   background (black), slot 14 white, etc.  pa_cloc overwrites slots
   1 and 2 from the primary/secondary clothing tables; slot 6 is
   overwritten by lcp_upal for the sickness skin. */
short   main_pal[16]           = {
        0x000, 0x442, 0x265, 0x754,
        0x310, 0x040, 0x754, 0x760,
        0x247, 0x631, 0x700, 0x333,
        0x555, 0x007, 0x777, 0x410
};

/* vdi_colt (Ghidra vdi_color_table @ 0x29b64): color_enum ->
   VDI-color permutation.  ROM data at 0x29b64 (verified via
   /read_memory) is {0,2,3,6,4,7,5,8,9,10,11,14,12,15,13,1} -- exactly
   TOS's default ST-low permutation from VDI-index to palette-slot.
   The game names its own colours by palette slot (see main_pal) and
   calls vsl_color(vdi_colt[color_enum]) so that after TOS's
   permutation the pen lands on palette slot `color_enum`.

   Byte-for-byte match to ROM.  With a properly-opened VDI workstation
   (LCP.PRG launched directly from the GEM desktop / Hatari --auto),
   TOS applies its default permutation and color_enum 13 (blue) ->
   vdi_colt[13] = 15 -> palette 13 = main_pal[13] = 0x007 blue.
   Launching via COMMAND.PRG leaves the workstation in a state that
   collapses vsl_color's colour arg into pen 15 (dark brown 0x410)
   regardless of index -- see the sc_sdtb comment. */
short   vdi_colt[16]            = {
        0,  2,  3,  6,  4,  7,  5,  8,
        9, 10, 11, 14, 12, 15, 13,  1
};

short   no_keyin          = NO;

/* ---- Hardware sprite double-buffer (SPRITE_HW_SLOTS) -------------------
   Two parallel state sets per hardware slot: `pe` = pending (what game
   logic queued for the next 8 Hz compositor tick) and `ac` = active
   (currently drawn on the visible frame).  Slot layout: 0/7 = dog
   (behind/in-front of LCP by Y depth), 3 = LCP body, 4 = LCP head,
   1..2 and 5..6 = door/object overlay slots. */
/* Sized SPRITE_HW_SLOTS_ALLOC (= HW_SLOT_NONE + 1), not SPRITE_HW_SLOTS:
   sp_upds parks HIDDEN sprites in the disabled slot HW_SLOT_NONE (9),
   and gameTick's carrying path / sp_ssco then index these arrays at [9]
   for a momentarily-hidden carried sprite.  Allocating the scratch slot
   keeps that write in-bounds; see the SPRITE_HW_SLOTS_ALLOC note in
   sprglobs.h.  Real-slot loops still bound by SPRITE_HW_SLOTS. */
/* Explicitly initialized, so it lands in DATA (all zeros) rather than
   as a .comm -- that is where LCP_STX has it. */
short   g_sepef[SPRITE_HW_SLOTS_ALLOC] = { 0 }; /* sprite_pending_flag */

short   g_hacur                         = 8;

/* Ghidra head_anim_target_state @ 0x29b98 = 8, head_anim_current @ 0x29b96 = 8,
   head_anim_mode @ 0x29b9a = -1 (HEAD_ANIM_DISABLED). */
short   g_hatas                         = 8;

short   g_hamod                         = HEAD_ANIM_DISABLED;

/*
 * dat_u1.c -- the initialized globals that belong to the stx_u1
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u1.
 */

/*
 * dat_u1.c -- the initialized globals that belong to the stx_u1
 * OBJECT, in LCP_STX data order.
 *
 * The 1985 sources declared their globals in the file that used them,
 * so each object's data segment is its own globals followed by the
 * string literals and switch tables its code emits.  The object that
 * owns a stretch of anonymous data is not a guess: a switch table's
 * relocation points into its own function, and a string is emitted in
 * the object that references it.  See CLAUDE.md, "DATA and BSS
 * layout".
 *
 * Not compiled standalone -- included by stx_u1.
 */

/* Two more bytes of -1 that nothing references, between g_hamod and
   g_trac (LCP_STX data 0x4c0).  Dead 1985 data that Alcyon still
   allocates. */
short   g_unus3                         = -1;

short   g_trac                  = ACTION_NONE;

short   lcp_recP              = 0;

short   lcp_tv                       = 0;

BOOL16  ph_call  = NO;

BOOL16  fire_act                = NO;

BOOL16  ph_ans     = NO;

BOOL16  lunT_trg      = NO;

BOOL16  dinT_trg     = NO;

BOOL16  wkT_trg  = NO;

BOOL16  bedT_trg         = NO;

/* ---- Body / carry frame tables (index = PLAYER_STATE) ------------------ */
/* body_frT (Ghidra 0x29BB2, 93 shorts):
   maps lcp_st -> body-frame index into body.lcp / body_shp.
   Values dumped via ghidra_scripts/DumpTable.java. */
short   body_frT[93] = {
         0,  1,  2,  3,  4,  1,  6,  7,     /*  0..7  */
        43,  9, 10, 11, 12, 20, 21, 22,     /*  8..15 */
        21, 13, 14, 15, 16, 17, 18, 19,     /* 16..23 */
        18, 23, 24, 25, 24, 27, 28, 29,     /* 24..31 */
        30, 31, 32, 33, 34, 35, 36, 37,     /* 32..39 */
        27, 38, 39, 40, 41, 42, 43, 44,     /* 40..47 */
        45, 46, 47, 48, 49, 50, 51, 52,     /* 48..55 */
        53, 54, 67, 68, 32, 69, 70, 71,     /* 56..63 */
        72, 73, 74, 75, 76, 77, 78, 79,     /* 64..71 */
        80, 81, 82, 83, 84, 85, 86, 87,     /* 72..79 */
        88, 89, 90, 91, 92, 93, 94, 95,     /* 80..87 */
        96, 97, 26,  5,  8                  /* 88..92 */
};

/* cy_frT (Ghidra 0x29C6C, 25 shorts):
   alternate arms-up frames used while carrying an object in
   walking states 0..24. */
short   cy_frT[25]      = {
        55, 56, 57, 58, 55, 56, 57, 58, 43, 63, 64, 65, 66, 59, 60, 61, 62,
        13, 14, 15, 16, 17, 18, 19, 18
};

/* Per-PLAYER_STATE horizontal offset for the head anchor.  Ghidra
   0x29C9E, actual size = 93 shorts (distance to next symbol
   hd_hgt @ 0x29D58 is 186 bytes).  The previous
   [109] port declaration overflowed into adjacent tables, giving
   wrong offsets for lcp_st 93..108. */
short   hd_xoff[93] = {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  6,
         6,  0, -1,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0
};

/* Per-PLAYER_STATE head Y contribution (subtracted from body top).
   Ghidra 0x29D58, 93 shorts (distance to hd_dang
   @ 0x29E12). */
short   hd_hgt[93] = {
        21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 18, 18, 18, 18, 17, 17,
        17, 21, 21, 18, 18, 18, 18, 18,
        18, 18, 17, 21, 21, 21, 21, 21,
        21, 21, 21, 20, 21, 21, 21, 21,
        21, 21, 21, 18, 21, 21, 21, 21,
         5,  5,  5,  5,  5, 19, 19, 21,
        21, 21, 21, 21, 21, 21, 21, 20,
        21, 21, 20, 20, 21, 21, 21, 21,
        20, 21, 20, 21, 21
};

/* Neutral head-facing angle per PLAYER_STATE (used by head_animate to
   pick the "resting" horizontal direction the head drifts toward).
   Ghidra 0x29E12, 93 shorts (distance to room_position_x_table @
   0x29ECC).  The previous [109] port declaration read into
   room_position_x_table for lcp_st 93..108, producing wrong
   head_sprite_frame values that showed up as broken head phases
   whenever the character entered a state past 92. */
short   hd_dang[93] = {
         2,  2,  2,  2,  2,  2,  2,  2,
         2,  2,  2,  2,  2,  4,  4,  4,
         4,  2,  2,  2,  2,  0,  0,  0,
         0,  3,  4,  4,  4,  4,  4,  4,
         4,  4,  0,  0,  0,  0,  4,  4,
         4,  4,  4,  0,  0,  0,  2,  2,
         2,  2,  2,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  4,  4,  4,
         2,  2,  2,  2,  2,  1,  4,  0,
         0,  0,  0,  4,  4,  4,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  4,  3,  1
};

/* g_rpxs[48]: X half-pixel coordinate per HOUSE_POS.
   Table value gets left-shifted by 1 at the call site to yield the
   full-pixel X (see hs_posXY).
   addr: g_rpxs at 0x019eb2 */
short   g_rpxs[48] = {
        /* Floor 3 -- top       0..15 */
         22,  36,  49,  55,  60,  56,  73,  96,
        106, 118, 113, 110, 131,  47, 133, 146,
        /* Floor 2 -- middle   16..31 */
         16,  40,  27,  31,  45,  55,  84, 100,
        111, 100, 109, 124, 134, 135, 144,  67,
        /* Floor 1 -- bottom   32..47 */
          8,   8,  12,  19,  40,  25,  54,  49,
         67,  70, 106, 110, 123, 132, 147, 140
};

/* g_rphs[48]: Y offset from floor baseline per HOUSE_POS.  There is
   no leading 140 "ground-floor sentinel" -- LCP_STX's table starts at
   9 and its data gap here is 96 bytes = 48 shorts. */
short   g_rphs[48] = {
          9,  14,   9,  10,  11,  14,  12,  13,
         12,  12,  12,   6,  15,  10,  14,   3,
          3,   3,   8,  15,  13,  13,  12,  13,
         14,  12,   8,  14,  13,  14,  13,   5,
          8,   3,  10,  13,  13,  14,  10,  14,
         14,  12,  13,   7,  14,  12,  13,   2
};

/* body_yof (Ghidra 0x29F8C, 109 shorts):
   Y anchor offset per lcp_st.  Verified against Ghidra dump. */
short   body_yof[109] = {
        -2, -2, -2, -1, -2, -2, -2, -1,     /*   0..7  */
        -2,  0,  0,  0,  0,  0,  0,  0,     /*   8..15 */
         0,  0,  0,  0,  0,  0,  0,  0,     /*  16..23 */
         0, -2, -2, -2, -2, -2,  0,  0,     /*  24..31 */
         0, -2, -2, -2, -2, -2, -2, -2,     /*  32..39 */
        -2, -2,  0, -6, -6, -6, -2, -6,     /*  40..47 */
        -6,  2,  1,  7, -7, -5, -5, -5,     /*  48..55 */
        -5, -5, -4, -1,  0, -2, -2, -2,     /*  56..63 */
        11, 11, 11, 11, 11, -1, -1, -7,     /*  64..71 */
        -7, -4, -7, -2, -2, -4, -2, -1,     /*  72..79 */
        -2, -2,  0,  0, -2, -2, -2, -2,     /*  80..87 */
        -3, -2, -3, -2, -2,  1,  2,  6,     /*  88..95 */
        11, 17, 20, 22, 26, 30, 33, 35,     /*  96..103 */
        46,  1, 11, 26, 35                  /* 104..108 */
};

/* Ghidra 0x2A066, actual size = 6 shorts (distance to
   stair_ty @ 0x2A072 is 12 bytes).  The last 2 entries
   my earlier port added (124, 137) were `stair_ty` and
   `stair_by` -- adjacent globals, not part of the
   waypoint table. */
short   stair_wp[6]    = { 170, 185, 133, 124, 182, 72 };

/* Middle-floor staircase-2 landing coordinates (top-of-flight X and Y).
   The middle-floor branch of lcp_flwp uses these to
   route through the between-floor landing instead of the raw
   stair_wp entries.  Values dumped from Ghidra data. */
/* Ghidra 0x2A072 / 0x2A074. */
short   stair_ty           = 124;

short   stair_by        = 137;

short   flr_cy[3]        = { 198, 135, 71 };

/* Ghidra `lcp_on_stairs_flag` (short, YES/NO).  YES while
   lcp_pathfind_one_step is inside a stair-traversal path; drives the
   stair-specific sprite-state sequence 9..24 and the wood-stairs SFX
   selection. */
short   lcp_stR              = 0;

/* ---- Floor geometry ---------------------------------------------------- */
/* Bottom Y of each floor (used by pathfinding to detect floor boundary).
   flr_by[0] = top floor, [1] = middle floor, [2] = bottom. */
/* Ghidra-verified: floor 1 (bottom) .. floor 3 (top).  Dumped from
   0x2A07E (bottom), 0x2A076 (center), 0x2A066 (waypoints). */
short   flr_by[3]        = { 202, 140, 77 };

/* sp_fidx (Ghidra sprite_file_index_table @ 0x2A084, 50 shorts):
   file-record index -> sprite_id slot to store its pointers in. */
short   sp_fidx[50] = {
        12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27,
        28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43,
        44,  9, 45, 46, 47, 48, 49,  3,
         4, 50,  7,  6, 51, 52, 53, 54,
         8, 55
};

/* ---- Dog sprite pointers / buffers ------------------------------------- */
/* g_dwanf (Ghidra dog_walk_anim_frames @ 0x2A0E8): 8 sprite ids the
   walk cycle rotates through in dg_mvAni. */
short   g_dwanf[8] = {
        SPRITE_DOG_WLK_R1, SPRITE_DOG_WLK_R2,
        SPRITE_DOG_WLK_R3, SPRITE_DOG_WLK_R4,
        SPRITE_DOG_WLK_R5, SPRITE_DOG_WLK_R7,
        SPRITE_DOG_WLK_R8, SPRITE_DOG_WLK_R9
};

/* PEx.LCP filename.  Ghidra pex_name @ 0x2a0f8 points to "pex.lcp"
   at 0x2a330 and main() mutates index 2 to select the character.
   Port stores the string as a mutable static char array. */
char *  pex_name                     = "PE0.LCP";
