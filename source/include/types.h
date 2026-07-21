/*
 * types.h -- primitive C types used across the reconstruction.
 *
 * Alcyon C 4.14 for the Atari ST: short = 16 bits, long = 32 bits,
 * char = 8-bit signed. Struct layout is packed; the compiler aligns
 * shorts on even boundaries but does not add trailing padding.
 *
 * addr: n/a (project-wide typedef)
 */

#ifndef TYPES_H
#define TYPES_H

/* Under Alcyon C 4.14, external identifiers truncate to 7 C-name chars
   at the linker layer.  alcyon_names.h aliases all colliding long
   names to unique short ones and also #defines `void` -> `int` (Alcyon
   has no `void` keyword).  Included transparently so every .c and
   every extern block sees the same mapping. */
#ifdef __ALCYON__
#include "alcnames.h"
#endif

typedef short           BOOL16;         /* YES / NO container */

#define YES     1
#define NO      0

/* NULL comes from <stddef.h> on any hosted C impl; only define here if
   the host headers haven't set it (Alcyon's <stddef.h> is minimal and
   sometimes absent from partial installs). */
#ifndef NULL
#define NULL    ((void *) 0)
#endif

#endif  /* TYPES_H */
