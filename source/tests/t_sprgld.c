/*
 * t_sprgld.c -- golden-master render test for the sprite compositor.
 *
 * Iterates animState = 0..29 (the animation range for which
 * bodyIndex has non-zero entries) x both facings, calling
 * updateBody() for each, and packs all 60 outputs into a
 * single 4-column x 15-row atlas PGM (4*64 = 256 wide, 15*21 = 315 tall).
 *
 * REFERENCE RE-BLESSED 2026-09-05.  The checked-in atlas was produced
 * by the retired LCP_ORG-era revision.  The sprite path is now the
 * byte-identical LCP_STX code -- updateBody was recovered byte for byte
 * and bodyFrames/bodyShapes became real arrays -- so the old master no
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

extern PLAYER   resident;
extern short    resX;
extern short    resY;
extern short    animState;
extern short    resFacing;
extern short    isCarrying;
extern short    debugHideLcp;
extern short    pendReady[];
/* bodyFrames and bodyShapes are real global ARRAYS in LCP_STX, not
   pointers -- updateBody indexes them with an immediate base and no
   ext.l, which is what pinned the shape (see docs/history.md).  So the
   frames are COPIED in here; there is nothing to re-point. */
extern unsigned char    bodyFrames[][LCP_BODY_FRAME_SIZE];
extern unsigned char    bodyShapes[][LCP_BODY_SHAPE_SIZE];
extern short    bodyImage[];
extern void     updateBody();
extern void     initMirror();

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
        int     pxX;

        for (row = 0; row < TILE_H; row++) {
                for (word = 0; word < 4; word++) {
                        v = (unsigned short) buf[row * 4 + word];
                        for (bit = 15; bit >= 0; bit--) {
                                pxX = word * 16 + (15 - bit);
                                dst[row * ATLAS_W + pxX]
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
        long            payloadBytes;
        unsigned char * bodyBuf;
        unsigned char * shapeBuf;
        int             i;
        int             tileIx;
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
        count = ((long) header[0] << 8) | header[1];
        payloadBytes = ((long) header[2] << 8) | header[3];
        bodyBuf = (unsigned char *) malloc(payloadBytes);
        if (bodyBuf == NULL) { perror("malloc"); return 1; }
        if ((long) fread(bodyBuf, 1, payloadBytes, f) != payloadBytes) {
                perror("payload"); return 1;
        }
        fclose(f);
        shapeBuf = (unsigned char *) calloc(1, payloadBytes);
        memcpy(bodyFrames, bodyBuf,
               (size_t) ((payloadBytes < 120L * 168L)
                         ? payloadBytes : 120L * 168L));
        memset(bodyShapes, 0, BODY_FRAMES * LCP_BODY_SHAPE_SIZE);   /* the whole array */

        /* mirrorTable is BSS in LCP_STX -- initMirror builds the
           bit-reversal LUT at boot (it used to be a shipped
           data table).  Without this the mirrored, right-facing
           frames come out blank. */
        initMirror();
        memset(&resident, 0, sizeof(resident));
        resX = 100;
        resY = 100;
        isCarrying = 0;
        debugHideLcp = 0;
        pendReady[3] = 0;

        memset(atlas, 255, sizeof(atlas));

        /* 60 renders: 30 states x 2 facings, laid out row-major into
           the ATLAS_COLS x ATLAS_ROWS grid. */
        tileIx = 0;
        for (i = 0; i < N_STATES; i++) {
                int facing;
                for (facing = 0; facing < 2; facing++) {
                        animState = i;
                        resFacing = facing;
                        memset(bodyImage, 0, LCP_BODY_DEST_WORDS * sizeof(short));
                        /* Clear the double-buffer flag every iteration:
                           updateBody sets it to YES on exit and
                           spin-waits for it to clear on entry; in-game
                           the render pipeline clears it, but in this
                           test we're the only thing running. */
                        pendReady[3] = 0;
                        updateBody();

                        row = tileIx / ATLAS_COLS;
                        col = tileIx % ATLAS_COLS;
                        render_tile(&atlas[row * TILE_H * ATLAS_W
                                           + col * TILE_W],
                                    bodyImage);
                        tileIx++;
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
                free(bodyBuf); free(shapeBuf);
                return 0;
        }

        /* Diff against reference. */
        {
                unsigned char   refHdr[64];
                unsigned char * refBuf;
                size_t          got;
                int             c;

                /* Skip the PGM header (3 whitespace-separated tokens
                   after "P5\n" -- width, height, maxval). */
                if (fread(refHdr, 1, 3, ref) != 3
                    || refHdr[0] != 'P' || refHdr[1] != '5') {
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
                refBuf = (unsigned char *) malloc(sizeof(atlas));
                got = fread(refBuf, 1, sizeof(atlas), ref);
                fclose(ref);
                if (got != sizeof(atlas)) {
                        printf("sprite_golden: reference size wrong "
                               "(read %lu, expected %lu)\n",
                               (unsigned long) got,
                               (unsigned long) sizeof(atlas));
                        return 1;
                }
                mismatch = memcmp(atlas, refBuf, sizeof(atlas));
                free(refBuf);
        }

        free(bodyBuf); free(shapeBuf);

        if (mismatch == 0) {
                printf("sprite_golden: PASS  (60 renders match reference)\n");
                return 0;
        }
        printf("sprite_golden: FAIL  (atlas differs from reference; "
               "inspect %s vs %s)\n", OUT_PATH, REF_PATH);
        return 1;
}
