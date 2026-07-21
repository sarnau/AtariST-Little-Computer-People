/* alcnames.h -- Alcyon C 4.14 compatibility shim.
 *
 * Included transparently from types.h under `#ifdef __ALCYON__`.
 *
 * All long external identifiers were renamed in-source to unique
 * 7-character forms in the Path B rename pass (see namemap.md), so
 * this file no longer aliases anything.  It only patches the one
 * keyword Alcyon C doesn't recognise: `void`.
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
