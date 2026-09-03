/*
 * parts/mq_intim.c -- shared body; LCP_STX puts mq_intim in the MIDI
 * object at 0x1112, between mq_stop and mq_extm.  Files under parts/
 * are never compiled standalone.
 */
/* mq_intim: in THIS ROM an empty stub (0x804e) -- no Xbtimer call
   exists anywhere in the binary; its ~1.5 KB music engine (0x8cce)
   runs without a Timer-A ISR.  The port KEEPS the other-image
   Timer-A sequencer for now (same policy as the minigames: retained
   working features), because the port's mq_* engine needs the ISR --
   without it a_plawr's wait-for-mi_play spins forever.  INTENTIONAL
   non-fidelity until the ROM's polled engine is recovered.
   addr: mq_intim() */

void
mq_intim()
{
#ifdef SKIP_MIDI
        /* Test builds: Timer-A jitter breaks frame-hash goldens. */
        (void) 0;
#else
        g_mtpre = 100;
        g_mtdiv = 4;
        mi_svtv = Setexc(0x4d, -1L);
        Xbtimer(0, 5, 0x28, (long) mq_tick);
#endif
}
