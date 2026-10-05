/* Load a .sng/.org from disk (10-byte Music Studio 2.0 header,
   then up to 20000 bytes of sequence data) and hand it to startSong. */


void
playSongFile(filename)
char *  filename;
{
        /* `unused` is never written but must stay: removing it changes
           the compiled code. */
        short           fhnd;
        short           unused;
        unsigned char   temp[10];
        _DTA *   dta_ptr;

        useSongChan = YES;
        fixedChan = YES;

        if (songPlaying != NO) {
                startSong(songBuf, songMaxPos);
                while (songPlaying != NO)
                        ;
        }
        if (songBuf != (char *) 0) {
                Mfree(songBuf);
                songBuf = (char *) 0;
        }

        Fsfirst(filename, F_NORMAL);
        dta_ptr = (_DTA *) Fgetdta();
        songBuf = (char *) Malloc(dta_ptr->d_length);
        if (songBuf == (char *) 0)
                outOfMemory();

        fhnd = openFile(filename, RMODE_RD);
        if (fhnd >= 0) {
                readFile(fhnd, 10L, temp);
                readFile(fhnd, 20000L, songBuf);
                Fclose(fhnd);
        }
        startSong(songBuf, songMaxPos);
}
