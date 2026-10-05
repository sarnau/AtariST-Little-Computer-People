/*
 * parts/hookTimerA.c -- included by midi_seq.c, between stopSequencer and
 * unhookTimerA; never compiled on its own.
 */
/* Install timerAIsr as the MFP Timer-A interrupt (prescaler /64, data
   0x28), saving the old vector first.  The sequencer is driven
   entirely by this interrupt: without it playOrgan's wait for mi_play
   never ends. */

void
hookTimerA()
{
#ifdef SKIP_MIDI
        /* Test builds: Timer-A jitter breaks frame-hash goldens. */
        (void) 0;
#else
        g_mtpre = 100;
        g_mtdiv = 4;
        mi_svtv = Setexc(VEC_TIMER_A, SETEXC_QUERY);
        Xbtimer(XB_TIMER_A, MFP_DIV64, 0x28, (long) timerAIsr);
#endif
}
