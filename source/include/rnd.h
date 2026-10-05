/* rnd.h -- the declaration of rnd(), kept out of protos.h on purpose.

   rnd() returns the long from the XBIOS Random() call.  The original
   declared it only in some of its source files: stepHead's unit never
   saw a declaration, so there the call is an implicit int and compiles
   to different (shorter) code.  Including this header everywhere would
   change that function's bytes, so only the units that had the
   declaration include it: stx_u1.c, stx_u2.c and games.c. */

#ifndef RND_H
#define RND_H

extern long rnd();

#endif /* RND_H */
