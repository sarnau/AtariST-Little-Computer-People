/*
 * parts/rnd.c -- a global wrapper around the raw XBIOS Random().  Most
 * callers go through it; rndRng inlines the trap instead.
 *
 * Included by stx_u1.c; never compiled on its own.
 */
long
rnd()
{
        return Random();
}
