/* alcnames.h -- Alcyon C 4.14 compatibility shim.
 *
 * Included transparently from types.h under `#ifdef __ALCYON__`.
 *
 * It patches the keywords Alcyon C doesn't recognise: `void` and
 * `volatile`.  It aliases no names: the linker keeps only eight
 * characters of a symbol (`_` + 7), so every external name is already
 * unique in its first seven characters in the source itself.
 */

#ifndef ALCNAMES_H
#define ALCNAMES_H

#ifdef __ALCYON__
#define void      int   /* Alcyon has no `void` keyword; use int for K&R */
#define volatile        /* Alcyon has no `volatile`; strip it */
#endif

#endif  /* ALCNAMES_H */
