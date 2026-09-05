/*
 * t_assets.c -- smoke test for the four asset loaders.
 *
 * ldObj() and ldSpr() slurp OBJECTS and SPRITES into obj_file[] and
 * spr_file[]; al_loal() reads a {count:BE16, total:BE16, payload}
 * file into a caller buffer.  None of them returns a record count any
 * more -- the loaders in LCP_STX just move bytes, and main() walks the
 * result afterwards -- so what there is to verify is that the bytes
 * arrive intact through fOpen/fr_read/Fclose.
 *
 * al_loal reads its two header words with raw two-byte fr_reads, so on
 * a little-endian host they come back byte-swapped and it would then
 * read the wrong length.  That is faithful ST code; the test writes a
 * host-endian copy of the header (payload untouched) so the loader's
 * logic is what is under test.  Same approach as t_sounds.c.
 *
 * Build: make assets_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/types.h"
#include "../include/structs.h"

extern void             ldObj();
extern void             ldSpr();
extern short            al_loal();
extern unsigned char    obj_file[];
extern unsigned char    spr_file[];

static int      fails;

/* Copy verbatim. */
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

/* Copy with the two header words rewritten in host byte order. */
static int
copy_hdr_swapped(src, dst, countp, totalp)
char *  src;
char *  dst;
short * countp;
short * totalp;
{
        FILE *          fi;
        FILE *          fo;
        unsigned char   hdr[4];
        unsigned char   buf[32768];
        size_t          n;
        short           count;
        short           total;

        fi = fopen(src, "rb");
        if (fi == NULL) { perror(src); return 1; }
        if (fread(hdr, 1, 4, fi) != 4) { fclose(fi); return 1; }
        count = (short) (((int) hdr[0] << 8) | hdr[1]);
        total = (short) (((int) hdr[2] << 8) | hdr[3]);
        fo = fopen(dst, "wb");
        if (fo == NULL) { perror(dst); fclose(fi); return 1; }
        fwrite(&count, sizeof count, 1, fo);
        fwrite(&total, sizeof total, 1, fo);
        while ((n = fread(buf, 1, sizeof buf, fi)) > 0)
                fwrite(buf, 1, n, fo);
        fclose(fi);
        fclose(fo);
        *countp = count;
        *totalp = total;
        return 0;
}

static long
file_size(path)
char *  path;
{
        FILE *  f;
        long    n;

        f = fopen(path, "rb");
        if (f == NULL) { perror(path); return 0L; }
        fseek(f, 0L, SEEK_END);
        n = ftell(f);
        fclose(f);
        return n;
}

/* Compare a loaded buffer against the file it came from. */
static void
check_bytes(what, got, path, offset, len)
char *          what;
unsigned char * got;
char *          path;
long            offset;
long            len;
{
        FILE *          f;
        unsigned char * want;
        long            n;

        want = (unsigned char *) malloc((size_t) len);
        if (want == NULL) { printf("  FAIL %s: out of memory\n", what); fails++; return; }
        f = fopen(path, "rb");
        if (f == NULL) { perror(path); free(want); fails++; return; }
        fseek(f, offset, SEEK_SET);
        n = (long) fread(want, 1, (size_t) len, f);
        fclose(f);
        if (n != len) {
                printf("  FAIL %s: only %ld of %ld bytes in the file\n",
                       what, n, len);
                fails++;
        } else if (memcmp(got, want, (size_t) len) != 0) {
                printf("  FAIL %s: %ld bytes loaded do not match the file\n",
                       what, len);
                fails++;
        } else {
                printf("  OK   %s: %ld bytes match the file\n", what, len);
        }
        free(want);
}

int
main()
{
        setvbuf(stdout, NULL, _IONBF, 0);

        if (copy_to_cwd("../../../DATA/OBJECTS", "objects")) return 1;
        if (copy_to_cwd("../../../DATA/SPRITES", "sprites")) return 1;

        /* Both loaders ask for a flat 14000 bytes; the files are
           smaller than that and fr_read simply returns what is there,
           so compare against each file's real length. */
        printf("OBJECTS -> obj_file[]\n");
        ldObj();
        check_bytes("ldObj", obj_file, "objects", 0L, file_size("objects"));

        printf("SPRITES -> spr_file[]\n");
        ldSpr();
        check_bytes("ldSpr", spr_file, "sprites", 0L, file_size("sprites"));

        {
                static unsigned char    buf[20160];
                short                   count;
                short                   total;

                if (copy_hdr_swapped("../../../DATA/BODY.LCP", "body.lcp",
                                     &count, &total)) return 1;
                printf("BODY.LCP: %d frames, %d payload bytes\n",
                       count, total);
                if (count != 98 || total != 16464) {
                        printf("  FAIL: header should be 98 frames / "
                               "16464 bytes\n");
                        fails++;
                }
                memset(buf, 0xaa, sizeof buf);
                al_loal("body.lcp", buf);
                check_bytes("al_loal BODY.LCP", buf,
                            "../../../DATA/BODY.LCP", 4L, (long) total);
        }

        {
                static unsigned char    buf[12288];
                short                   count;
                short                   total;

                if (copy_hdr_swapped("../../../DATA/PE2.LCP", "pe2.lcp",
                                     &count, &total)) return 1;
                printf("PE2.LCP: %d frames, %d payload bytes\n",
                       count, total);
                if (count != 66 || total != 11088) {
                        printf("  FAIL: header should be 66 frames / "
                               "11088 bytes\n");
                        fails++;
                }
                memset(buf, 0xaa, sizeof buf);
                al_loal("pe2.lcp", buf);
                check_bytes("al_loal PE2.LCP", buf,
                            "../../../DATA/PE2.LCP", 4L, (long) total);
        }

        if (fails) {
                printf("FAIL: %d asset check(s) failed\n", fails);
                return 1;
        }
        printf("PASS: all four asset loaders move the bytes intact\n");
        return 0;
}
