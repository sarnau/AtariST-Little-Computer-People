/*
 * parts/rnd.c -- the raw XBIOS Random() behind a global wrapper at
 * 0x69c6 (seven jsr call sites; rndRng inlines the trap instead).
 * Files under parts/ are never compiled standalone.
 */
long
rnd()
{
        return Random();
}
