/*
 * parts/mq_intim.c -- included by midi_seq.c, between mq_stop and
 * mq_extm; never compiled on its own.
 */
/* Install mq_tick as the MFP Timer-A interrupt (prescaler /64, data
   0x28), saving the old vector first.  The sequencer is driven
   entirely by this interrupt: without it a_plawr's wait for mi_play
   never ends. */

void
mq_intim()
{
#ifdef SKIP_MIDI
        /* Test builds: Timer-A jitter breaks frame-hash goldens. */
        (void) 0;
#else
        g_mtpre = 100;
        g_mtdiv = 4;
        mi_svtv = Setexc(VEC_TIMER_A, SETEXC_QUERY);
        Xbtimer(XB_TIMER_A, MFP_DIV64, 0x28, (long) mq_tick);
#endif
}
