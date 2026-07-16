/*
 * savehost.c -- HOST-only GEMDOS shim used by save.c during host builds.
 *
 * On the Atari ST the original code calls _gemdos(fn, ...) which
 * expands to a trap #1 with the function number in D0.  On the host we
 * route the file-I/O subset (Fopen/Fread/Fwrite/Fclose/Fcreate) through
 * stdio so save.c can round-trip a real HYBER file to disk without
 * needing an emulator.
 *
 * Only present when -DHOST is on; when building under Alcyon this file
 * is dropped from the SOURCES list and the real trap #1 is used.
 */

#ifdef HOST

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>             /* malloc / free */
#include <string.h>             /* memset for Fsfirst DTA scratch */
#include "include/enums.h"

/* K&R style default argument promotions mangle short/long arg passing
   on 64-bit hosts (a `short` promoted to `int` won't fill a `long`
   parameter slot).  Force a proper ANSI prototype so fr_read's
   `_gemdos(GEMDOS_Fread, handle, count, buf)` call passes arguments in
   the correct sizes.  On the ST side, TRAP #1 handles the ABI directly. */
long hst_gem(short fn, long a, long b, long c);

/* Simple handle table so we can pass a small int back to caller and
   fish out the underlying FILE * on each subsequent call. */
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

/* hst_gem: variadic wrapper so we can dispatch on function
   number.  Called from save.c via the _gemdos macro. */

long
hst_gem(short fn, long a, long b, long c)
{
        FILE *          fp;
        char *          path;
        short           handle;
        size_t          n;

        switch (fn) {
        case GEMDOS_Fopen:
                path = (char *) a;
                fp = fopen(path, b ? "wb" : "rb");
                if (fp == NULL)
                        return -1;
                return al_hnd(fp);

        case GEMDOS_Fcreate:
                path = (char *) a;
                fp = fopen(path, "wb");
                if (fp == NULL)
                        return -1;
                return al_hnd(fp);

        case GEMDOS_Fread:
                handle = (short) a;
                if (handle < 0 || handle >= MAX_HOST_HANDLES ||
                    host_handles[handle] == NULL)
                        return -1;
                n = fread((void *) c, 1, (size_t) b, host_handles[handle]);
                return (long) n;

        case GEMDOS_Fwrite:
                handle = (short) a;
                if (handle < 0 || handle >= MAX_HOST_HANDLES ||
                    host_handles[handle] == NULL)
                        return -1;
                n = fwrite((void *) c, 1, (size_t) b, host_handles[handle]);
                return (long) n;

        case GEMDOS_Fclose:
                handle = (short) a;
                if (handle < 0 || handle >= MAX_HOST_HANDLES)
                        return -1;
                if (host_handles[handle] != NULL) {
                        fclose(host_handles[handle]);
                        host_handles[handle] = NULL;
                }
                return 0;

        case GEMDOS_Malloc:
                /* GEMDOS Malloc(bytes) returns a pointer or 0 on OOM.
                   `a` carries the requested size. */
                if (a <= 0)
                        return 0;
                return (long) malloc((size_t) a);

        case GEMDOS_Mfree:
                /* Mfree(ptr).  `a` is the pointer to release. */
                if (a != 0)
                        free((void *) a);
                return 0;

        case GEMDOS_Fgetdta:
                /* We don't emulate the disk-transfer-area directory
                   iterator here; just return a pointer to a static
                   scratch buffer so callers don't NPE.  Fsfirst/Fsnext
                   are also unimplemented and return -1. */
                {
                        static char host_dta[64];
                        return (long) host_dta;
                }

        case GEMDOS_Fsfirst:
        case GEMDOS_Fsnext:
                return -1;      /* no directory iteration on host */
        }
        return -1;
}

/* XBIOS trap #14 dispatcher.  We route the handful of functions
   actually called (Random, Setpalette, Setscreen, Logbase, Giaccess)
   through their existing host-side equivalents; anything else returns
   0.  Called via the _xbios macro in osbind.h. */

long
hst_xb(short fn, long a, long b, long c)
{
        (void) b;
        (void) c;
        switch (fn) {
        case 17:                                /* Random */
                return (long) rand();
        case 6:                                 /* Setpalette */
                (void) a;
                return 0;
        case 5:                                 /* Setscreen */
                (void) a;
                return 0;
        case 3:                                 /* Logbase */
                return 0;
        case 28:                                /* Giaccess */
                return 0;
        }
        return 0;
}

#endif  /* HOST */
