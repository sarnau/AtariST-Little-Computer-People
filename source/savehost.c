#include "savehost.h"
/*
 * savehost.c -- HOST-only osbind stubs.
 *
 * On the Atari ST the port calls Fopen/Fread/Fwrite/Fclose/... which
 * expand (via <osbind.h>) to a real trap #1.  On the host we route
 * the file-I/O subset through stdio and provide plain-C stubs for
 * the memory/console entry points so save.c can round-trip a real
 * HYBER file to disk without needing an emulator.
 *
 * Only present when -DHOST is on; when building under Alcyon this
 * file is dropped from the SOURCES list and the real trap #1 is used.
 */

#ifdef HOST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Small handle table so callers get a short back and we look the
   FILE * up on each subsequent call. */
#define MAX_HOST_HANDLES        16
static FILE *   host_handles[MAX_HOST_HANDLES];

static short
al_hnd(fp)
FILE *  fp;
{
        short   i;
        for (i = 0; i < MAX_HOST_HANDLES; i = i + 1) {
                if (host_handles[i] == NULL) {
                        host_handles[i] = fp;
                        return i;
                }
        }
        fclose(fp);
        return -1;
}

/* --- GEMDOS trap #1 subset ------------------------------------------- */

short
Fopen(path, mode)
char *  path;
short   mode;
{
        FILE *  fp = fopen(path, mode ? "wb" : "rb");
        if (fp == NULL) return -1;
        return al_hnd(fp);
}

short
Fcreate(path, attr)
char *  path;
short   attr;
{
        FILE *  fp = fopen(path, "wb");
        (void) attr;
        if (fp == NULL) return -1;
        return al_hnd(fp);
}

long
Fread(handle, count, buf)
short   handle;
long    count;
void *  buf;
{
        if (handle < 0 || handle >= MAX_HOST_HANDLES ||
            host_handles[handle] == NULL) return -1;
        return (long) fread(buf, 1, (size_t) count, host_handles[handle]);
}

/* GEMDOS Fseek(offset, handle, mode): 0 = from start, 1 = from the
   current position, 2 = from the end.  Returns the new position. */
long
Fseek(offset, handle, mode)
long    offset;
short   handle;
short   mode;
{
        int     whence;

        if (handle < 0 || handle >= MAX_HOST_HANDLES ||
            host_handles[handle] == NULL) return -1;
        whence = (mode == 1) ? SEEK_CUR : (mode == 2) ? SEEK_END : SEEK_SET;
        if (fseek(host_handles[handle], (long) offset, whence) != 0)
                return -1;
        return (long) ftell(host_handles[handle]);
}

long
Fwrite(handle, count, buf)
short   handle;
long    count;
void *  buf;
{
        if (handle < 0 || handle >= MAX_HOST_HANDLES ||
            host_handles[handle] == NULL) return -1;
        return (long) fwrite(buf, 1, (size_t) count, host_handles[handle]);
}

short
Fclose(handle)
short   handle;
{
        if (handle < 0 || handle >= MAX_HOST_HANDLES) return -1;
        if (host_handles[handle] != NULL) {
                fclose(host_handles[handle]);
                host_handles[handle] = NULL;
        }
        return 0;
}

/* GEMDOS Malloc/Mfree, with one behaviour of the real thing that the
   host libc does not share: TOS looks the pointer up in its own block
   list and ignores anything it did not hand out.  The port relies on
   that -- fr_reac frees the buffer pointer its nibble loop has already
   walked forward (LCP_STX has no saved copy, see CLAUDE.md), which is
   harmless on the ST and an abort() on macOS.  So Mfree frees only
   blocks Malloc actually returned. */
#define MAX_HOST_BLOCKS 64
static void *   host_blocks[MAX_HOST_BLOCKS];

void *
Malloc(sz)
long    sz;
{
        void *  p;
        int     i;

        if (sz <= 0) return NULL;
        p = malloc((size_t) sz);
        if (p != NULL)
                for (i = 0; i < MAX_HOST_BLOCKS; i++)
                        if (host_blocks[i] == NULL) {
                                host_blocks[i] = p;
                                break;
                        }
        return p;
}

long
Mfree(p)
void *  p;
{
        int     i;

        if (p == NULL) return 0;
        for (i = 0; i < MAX_HOST_BLOCKS; i++)
                if (host_blocks[i] == p) {
                        host_blocks[i] = NULL;
                        free(p);
                        return 0;
                }
        return 0;               /* not ours: TOS would ignore it too */
}

void *
Fgetdta()
{
        static char host_dta[64];
        return host_dta;
}

short Fsfirst(pat, attr)  char *pat; short attr; { (void)pat; (void)attr; return -1; }
short Fsnext()                                    { return -1; }
short Cconis()                                    { return 0; }
long  Crawcin()                                   { return 0; }
void *Super(ssp)      void *ssp; { (void) ssp; return NULL; }
short Dsetpath(p)     char *p;   { (void) p;   return 0; }

#endif  /* HOST */
