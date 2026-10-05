/* Stop a currently-playing record so the resident can start
   writing/typing: walks to the dance floor, drains the MIDI buffer and
   frees it. */
void
stopRecord()
{
        /* No local: the walk result is tested in place. */

        if (recordPlaying == NO)
                return;

        posToXY(POS_TOP_DANCE_FLOOR, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        gameTick(2);

        if (songPlaying != NO) {
                startSong(songBuf, songMaxPos);
                while (songPlaying != NO)
                        ;
        }
        recordStoop();
        recordPlaying = NO;
        if (songBuf != (char *) 0) {
                Mfree(songBuf);
                songBuf = (char *) 0;
        }
}
