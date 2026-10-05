/* playRecord: pick a random .sng file and start it playing.
   Uses foodSupply as a modulo index (the 1985 code reused the field). */

void
playRecord()
{
        /* Three locals: a temporary, index (reused as the '.' scan
           counter) and the name pointer. */
        short   tmp;
        short   index;
        char *  filename;

        if (recordPlaying != NO)
                return;

        posToXY(POS_TOP_DANCE_FLOOR, &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        gameTick(2);
        recordStoop();
        recordPlaying = YES;

        tmp = rndRng(0, foodSupply - 1);
        index = tmp + 1;
        Fsfirst("*.sng", F_NORMAL);
        while (--index != 0)
                Fsnext();
        filename = ((_DTA *) Fgetdta())->d_fname;
        for (index = 0; filename[index] != '.'; index++)
                ;
        filename[index + 4] = '\0';
        playSongFile(filename);
}
