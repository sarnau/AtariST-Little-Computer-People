/*
 * sprite_golden.c -- golden-master render test for the sprite compositor.
 *
 * Iterates lcp_st = 0..29 (the animation range for which
 * body_sprite_frame_table has non-zero entries) x both facings, calling
 * sp_updb() for each, and packs all 60 outputs into a
 * single 4-column x 15-row atlas PGM (4*64 = 256 wide, 15*21 = 315 tall).
 *
 * REFERENCE RE-BLESSED 2026-09-05.  The checked-in atlas was produced
 * by the retired LCP_ORG-era revision.  The sprite path is now the
 * byte-identical LCP_STX code -- sp_updb was recovered byte for byte
 * and body_ptr/body_shp became real arrays -- so the old master no
 * longer describes what the original computes.  The new one was
 * eyeballed (60 tiles, both facings populated) before being blessed.
 *
 * The atlas is written to sprite_golden.pgm.  If tests/reference/
 * sprite_golden.pgm exists, the test byte-diffs the two and fails on
 * any mismatch -- catching sprite pipeline regressions.  If the
 * reference file doesn't exist, the test writes it into place, prints
 * a message asking the reviewer to inspect it, and exits successfully
 * so the initial run bootstraps the reference.
 *
 * Build: make sprite_golden_test
 * Run:   from source/build/host/, execute ./sprite_golden_test
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/types.h"
#include "../include/structs.h"
#include "../include/enums.h"
#include "../include/sprites.h"

extern PLAYER   lcp;
extern short    lcp_x;
extern short    lcp_y;
extern short    lcp_st;
extern short    lcp_face;
extern short    g_lcyof;
extern short    dbg_hide;
extern short    g_sepef[];
/* body_ptr and body_shp are real global ARRAYS in LCP_STX, not
   pointers -- sp_updb indexes them with an immediate base and no
   ext.l, which is what pinned the shape (see CLAUDE.md).  So the
   frames are COPIED in here; there is nothing to re-point. */
extern unsigned char    body_ptr[][168];
extern unsigned char    body_shp[][84];
extern short    g_lsimg[];
extern void     sp_updb();
extern void     initBRev();

#define N_STATES        30
#define ATLAS_COLS      4
#define ATLAS_ROWS      15      /* 2 facings * 30 / 4 cols */
#define TILE_W          64      /* 4 words per row */
#define TILE_H          21
#define ATLAS_W         (ATLAS_COLS * TILE_W)
#define ATLAS_H         (ATLAS_ROWS * TILE_H)

/* Reference / output paths (relative to build/host/ where the binary
   runs).  The reference lives under tests/reference/ so it's checked
   into the repo alongside the test source. */
#define OUT_PATH        "sprite_golden.pgm"
#define REF_PATH        "../../tests/reference/sprite_golden.pgm"

static unsigned char    atlas[ATLAS_H * ATLAS_W];

static void
render_tile(dst, buf)
unsigned char * dst;
short *         buf;
{
        int     row;
        int     word;
        int     bit;
        unsigned short v;
        int     px_x;

        for (row = 0; row < TILE_H; row++) {
                for (word = 0; word < 4; word++) {
                        v = (unsigned short) buf[row * 4 + word];
                        for (bit = 15; bit >= 0; bit--) {
                                px_x = word * 16 + (15 - bit);
                                dst[row * ATLAS_W + px_x]
                                        = (v >> bit) & 1 ? 0 : 255;
                        }
                }
        }
}

int
main(argc, argv)
int     argc;
char ** argv;
{
        FILE *          f;
        unsigned char   header[4];
        long            count;
        long            payload_bytes;
        unsigned char * body_buf;
        unsigned char * shape_buf;
        int             i;
        int             tile_ix;
        int             row;
        int             col;
        FILE *          ref;
        int             mismatch;

        (void) argc;
        (void) argv;

        /* Load BODY.LCP. */
        f = fopen("../../../DATA/BODY.LCP", "rb");
        if (f == NULL) { perror("open DATA/BODY.LCP"); return 1; }
        if (fread(header, 1, 4, f) != 4) { perror("hdr"); return 1; }
        count         = ((long) header[0] << 8) | header[1];
        payload_bytes = ((long) header[2] << 8) | header[3];
        body_buf = (unsigned char *) malloc(payload_bytes);
        if (body_buf == NULL) { perror("malloc"); return 1; }
        if ((long) fread(body_buf, 1, payload_bytes, f) != payload_bytes) {
                perror("payload"); return 1;
        }
        fclose(f);
        shape_buf = (unsigned char *) calloc(1, payload_bytes);
        memcpy(body_ptr, body_buf,
               (size_t) ((payload_bytes < 120L * 168L)
                         ? payload_bytes : 120L * 168L));
        memset(body_shp, 0, 98 * 84);   /* the whole array */

        /* rev_tab is BSS in LCP_STX -- initBRev builds the
           bit-reversal LUT at boot (it used to be a shipped
           data table).  Without this the mirrored, right-facing
           frames come out blank. */
        initBRev();
        memset(&lcp, 0, sizeof(lcp));
        lcp_x                    = 100;
        lcp_y                    = 100;
        g_lcyof = 0;
        dbg_hide = 0;
        g_sepef[3]   = 0;

        memset(atlas, 255, sizeof(atlas));

        /* 60 renders: 30 states x 2 facings, laid out row-major into
           the ATLAS_COLS x ATLAS_ROWS grid. */
        tile_ix = 0;
        for (i = 0; i < N_STATES; i++) {
                int facing;
                for (facing = 0; facing < 2; facing++) {
                        lcp_st            = i;
                        lcp_face = facing;
                        memset(g_lsimg, 0, LCP_BODY_DEST_WORDS * sizeof(short));
                        /* Clear the double-buffer flag every iteration:
                           sp_updb sets it to YES on exit and
                           spin-waits for it to clear on entry; in-game
                           the render pipeline clears it, but in this
                           test we're the only thing running. */
                        g_sepef[3] = 0;
                        sp_updb();

                        row = tile_ix / ATLAS_COLS;
                        col = tile_ix % ATLAS_COLS;
                        render_tile(&atlas[row * TILE_H * ATLAS_W
                                           + col * TILE_W],
                                    g_lsimg);
                        tile_ix++;
                }
        }
        (void) count;

        /* Write the atlas. */
        f = fopen(OUT_PATH, "wb");
        if (f == NULL) { perror(OUT_PATH); return 1; }
        fprintf(f, "P5\n%d %d\n255\n", ATLAS_W, ATLAS_H);
        fwrite(atlas, 1, sizeof(atlas), f);
        fclose(f);

        /* Compare against the reference, or seed it on first run. */
        ref = fopen(REF_PATH, "rb");
        if (ref == NULL) {
                /* No reference yet: seed it. */
                ref = fopen(REF_PATH, "wb");
                if (ref == NULL) {
                        perror(REF_PATH);
                        printf("sprite_golden: could not seed reference "
                               "(mkdir tests/reference/?)\n");
                        return 1;
                }
                fprintf(ref, "P5\n%d %d\n255\n", ATLAS_W, ATLAS_H);
                fwrite(atlas, 1, sizeof(atlas), ref);
                fclose(ref);
                printf("sprite_golden: SEEDED %s from this run.  "
                       "Inspect it, then re-run to verify.\n", REF_PATH);
                free(body_buf); free(shape_buf);
                return 0;
        }

        /* Diff against reference. */
        {
                unsigned char   ref_hdr[64];
                unsigned char * ref_buf;
                size_t          got;
                int             c;

                /* Skip the PGM header (3 whitespace-separated tokens
                   after "P5\n" -- width, height, maxval). */
                if (fread(ref_hdr, 1, 3, ref) != 3
                    || ref_hdr[0] != 'P' || ref_hdr[1] != '5') {
                        printf("sprite_golden: reference has bad magic\n");
                        return 1;
                }
                /* We already consumed "P5\n" (3 bytes, 1 newline).
                   The width/height and maxval lines add 2 more
                   newlines before the binary payload starts. */
                {
                        int newlines = 0;
                        while (newlines < 2 && (c = fgetc(ref)) != EOF) {
                                if (c == '\n') newlines++;
                        }
                }
                ref_buf = (unsigned char *) malloc(sizeof(atlas));
                got = fread(ref_buf, 1, sizeof(atlas), ref);
                fclose(ref);
                if (got != sizeof(atlas)) {
                        printf("sprite_golden: reference size wrong "
                               "(read %lu, expected %lu)\n",
                               (unsigned long) got,
                               (unsigned long) sizeof(atlas));
                        return 1;
                }
                mismatch = memcmp(atlas, ref_buf, sizeof(atlas));
                free(ref_buf);
        }

        free(body_buf); free(shape_buf);

        if (mismatch == 0) {
                printf("sprite_golden: PASS  (60 renders match reference)\n");
                return 0;
        }
        printf("sprite_golden: FAIL  (atlas differs from reference; "
               "inspect %s vs %s)\n", OUT_PATH, REF_PATH);
        return 1;
}
