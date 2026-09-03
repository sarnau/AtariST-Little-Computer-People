/*
 * parts/p_dobls.c -- shared body; LCP_STX links it in at 0x15f9a,
 * immediately after deal_kc. Files under parts/ are never compiled
 * standalone.
 */
void p_dobls() { sf_sele(SFX_DOORBELL,  4L); }
