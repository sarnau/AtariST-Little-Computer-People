/*
 * parts/rnd.c -- LCP_STX has the raw XBIOS Random() behind a global
 * wrapper at 0x69c6 (seven jsr call sites; rndRng inlines the trap
 * instead).  LCP_ORG has no such function, so only the STX
 * configuration includes this body.  Files under parts/ are never
 * compiled standalone.
 */
long
rnd()
{
        return Random();
}
