/*
 * parts/initBRev.c -- LCP_STX 0x6804.  A ten-byte wrapper whose only
 * job is to call the bit-reversal table builder that follows it.
 * Files under parts/ are never compiled standalone.
 */
void
initBRev()
{
        rv_bld();
}
