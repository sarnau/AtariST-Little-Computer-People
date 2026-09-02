/*
 * parts/p_dobls.c -- shared body; LCP_ORG links it in sound.o,
 * LCP_STX at 0x15f9a, immediately after deal_kc.  Files under
 * parts/ are never compiled standalone.
 */
void p_dobls() { sf_sele(SFX_DOORBELL,  4L); }
