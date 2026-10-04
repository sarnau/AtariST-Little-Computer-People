/* alcnames.h -- Alcyon C 4.14 compatibility shim.
 *
 * Included transparently from types.h under `#ifdef __ALCYON__`.
 *
 * Long external identifiers are spelled as unique 7-character names in
 * the source itself (see namemap.md), so this file aliases nothing.  It
 * only patches the keywords Alcyon C doesn't recognise: `void` and
 * `volatile`.
 *
 * NOTE: cp68's macro-name table truncates to 8 characters, so any
 * long-name `#define` alias would collapse into an unintended
 * catch-all matching every identifier that shares its first 8 chars.
 * That's why identifier renames MUST happen in source, not via
 * macros.  `void` (4 chars) is short enough to be safe.
 */

#ifndef ALCNAMES_H
#define ALCNAMES_H

#ifdef __ALCYON__
#define void      int   /* Alcyon has no `void` keyword; use int for K&R */
#define volatile        /* Alcyon has no `volatile`; strip it */
#endif

#endif  /* ALCNAMES_H */
