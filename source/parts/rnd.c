/*
 * A global wrapper around the raw XBIOS Random(). Most callers go
 * through it; rndRng inlines the trap instead.
 */
long
rnd()
{
        return Random();
}
