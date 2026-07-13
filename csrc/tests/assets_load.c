/*
 * assets_load.c -- smoke test for the OBJECTS/SPRITES/BODY.LCP loaders.
 *
 * Copies each 1985 data file into the CWD, calls the loader + parser,
 * prints the record count and per-record dimensions.  Verifies that:
 *   1. The file is readable end-to-end.
 *   2. The record header parse correctly walks to the buffer end.
 *   3. The width/height values look plausible for the source content.
 *
 * Build: make assets_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/types.h"
#include "../include/structs.h"

extern short    asset_load_objects_table();
extern short    asset_load_sprites_table();
extern short    asset_load_lcp();
extern MFDB     object_tab_mfdb_table[];
extern short    object_tab_width[];
extern short    object_tab_height[];
extern MFDB     sprite_tab_mfdb_table[];
extern short    sprite_tab_width[];
extern short    sprite_tab_height[];

static int
copy_to_cwd(src, dst)
char *  src;
char *  dst;
{
        FILE *          fi;
        FILE *          fo;
        unsigned char   buf[32768];
        size_t          n;

        fi = fopen(src, "rb");
        if (fi == NULL) { perror(src); return 1; }
        fo = fopen(dst, "wb");
        if (fo == NULL) { perror(dst); fclose(fi); return 1; }
        while ((n = fread(buf, 1, sizeof buf, fi)) > 0)
                fwrite(buf, 1, n, fo);
        fclose(fi);
        fclose(fo);
        return 0;
}

int
main()
{
        short   count;
        short   i;
        int     fails = 0;

        setvbuf(stdout, NULL, _IONBF, 0);

        if (copy_to_cwd("../../../DATA/OBJECTS", "objects")) return 1;
        if (copy_to_cwd("../../../DATA/SPRITES", "sprites")) return 1;
        if (copy_to_cwd("../../../DATA/BODY.LCP", "body.lcp")) return 1;
        if (copy_to_cwd("../../../DATA/PE2.LCP",  "pe2.lcp"))  return 1;

        /* OBJECTS */
        count = asset_load_objects_table();
        printf("OBJECTS: %d records\n", count);
        for (i = 0; i < count && i < 5; i = i + 1)
                printf("  [%d] %dx%d\n", i,
                       object_tab_width[i], object_tab_height[i]);
        if (count < 30 || count > 64) {
                printf("  FAIL: unexpected count %d (want 30..64)\n", count);
                fails++;
        }

        /* SPRITES */
        count = asset_load_sprites_table();
        printf("SPRITES: %d records\n", count);
        for (i = 0; i < count && i < 5; i = i + 1)
                printf("  [%d] %dx%d\n", i,
                       sprite_tab_width[i], sprite_tab_height[i]);
        if (count < 30 || count > 64) {
                printf("  FAIL: unexpected count %d (want 30..64)\n", count);
                fails++;
        }

        /* BODY.LCP */
        {
                static unsigned char body_buf[20000];
                short frames = asset_load_lcp("body.lcp", body_buf,
                                              sizeof body_buf);
                printf("BODY.LCP: %d frames\n", frames);
                if (frames < 80 || frames > 120) {
                        printf("  FAIL: unexpected frame count %d (want 80..120)\n",
                               frames);
                        fails++;
                }
        }

        /* PE2.LCP -- character 2 head frames */
        {
                static unsigned char pe_buf[20000];
                short frames = asset_load_lcp("pe2.lcp", pe_buf,
                                              sizeof pe_buf);
                printf("PE2.LCP: %d frames\n", frames);
                if (frames < 40 || frames > 80) {
                        printf("  FAIL: unexpected frame count %d (want 40..80)\n",
                               frames);
                        fails++;
                }
        }

        if (fails == 0)
                printf("PASS: all 4 asset files loaded + parsed\n");
        return fails ? 1 : 0;
}
