/* Install timerAIsr as the MFP Timer-A interrupt (prescaler /64, data
   0x28: 2.4576 MHz / 64 / 40 = 960 Hz), saving the old vector first.  The sequencer is driven
   entirely by this interrupt: without it playOrgan's wait for songPlaying
   never ends. */

void
hookTimerA()
{
#ifdef SKIP_MIDI
        /* Test builds: Timer-A jitter breaks frame-hash goldens. */
        (void) 0;
#else
        seqCountdown = 100;
        envDivider = 4;
        oldTimerAVec = Setexc(VEC_TIMER_A, SETEXC_QUERY);
        Xbtimer(XB_TIMER_A, MFP_DIV64, 0x28, (long) timerAIsr);
#endif
}
