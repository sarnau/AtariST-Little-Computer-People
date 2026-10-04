/*
 * A wrapper whose only job is to call the bit-reversal table builder
 * that follows it.
 * Included by stx_u1.c; never compiled on its own.
 */
void
initBRev()
{
        rv_bld();
}
