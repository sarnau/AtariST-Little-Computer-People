/* playOrgan: play the organ (ACTION_PLAY_ORGAN).  Any record playing is
   stopped first (stopRecord).  The resident walks to the organ on the top
   floor (POS_TOP_ORGAN), a prop sprite is shown on the instrument, and a
   random *.ORG file is picked and started with playSongFile.  While it plays he switches
   to a new random reaching pose whenever any PSG channel's volume
   rises, so he moves in time with the notes.  organPlaying is set
   throughout to keep animRecPlayer's record-player animation still, and the
   song buffer is freed at the end. */
void
playOrgan()
{
        /* The walk result is tested in place, with no local for it.
           The declaration order of these locals sets their stack
           slots and must not change; xres is unused but must stay. */
        unsigned char   psg_a, psg_b, psg_c;
        unsigned char   prev_a, prev_b, prev_c;
        short           i;
        char *          filename;
        _DTA *           dta_ptr;
        long            xres;

        scratchArr[0] = STATE_ORGAN_REACH_R;
        scratchArr[1] = STATE_ORGAN_IDLE;
        scratchArr[2] = STATE_ORGAN_REACH_L;
        scratchArr[3] = STATE_ORGAN_PULL_OUT;

        prev_a = 0;
        prev_b = 0;
        prev_c = 0;
        noPreempt = YES;
        if (recordPlaying != NO)
                stopRecord();
        noPreempt = NO;

        posToXY(POS_TOP_ORGAN,
                              &walkXTarget, &walkYTarget);
        if (walkToTarget() != 0)
                return;

        organPlaying = YES;
        headMode = HEAD_ANIM_DISABLED;
        resFacing = FACING_RIGHT;
        animState = STATE_STAND_FACING_SCREEN;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        waitHeadTurn();
        gameTick(4);

        animState = STATE_ORGAN_REACH_R;
        spriteLayer[SPRITE_ORGAN_PROP] = SPRITE_IN_FRONT;
        activateSprite(SPRITE_ORGAN_PROP);
        pendX[spriteSlot[SPRITE_ORGAN_PROP]] = 146;
        pendY[spriteSlot[SPRITE_ORGAN_PROP]] =  54;
        gameTick(1);

        i = rndRng(1, organCount);
        Fsfirst("*.org", F_NORMAL);
        while (--i != 0)
                Fsnext();
        dta_ptr = (_DTA *) Fgetdta();
        filename = dta_ptr->d_fname;
        for (i = 0; filename[i] != '.'; i++)
                ;
        filename[i + 4] = '\0';
        playSongFile(filename);

        headMode = HEAD_ANIM_WALKING;
        while (songPlaying == NO)
                ;

        while (songPlaying != NO) {
                /* Plain word arguments here (no 0L), unlike stopSfx's
                   Giaccess writes: the argument shape changes the code. */
                psg_a = Giaccess(0, PSG_VOL_A) & 0x1f;
                psg_b = Giaccess(0, PSG_VOL_B) & 0x1f;
                psg_c = Giaccess(0, PSG_VOL_C) & 0x1f;

                animState = scratchArr[0];
                if (psg_a > prev_a || psg_b > prev_b || psg_c > prev_c) {
                        i = rndRng(1, 3);
                        while (scratchArr[i] == animState)
                                i = rndRng(1, 3);
                        animState = scratchArr[i];
                        if (scratchArr[3] == animState) {
                                gameTick(0);
                                animState = scratchArr[rndRng(1, 2)];
                        }
                }
                prev_a = psg_a; prev_b = psg_b; prev_c = psg_c;
                gameTick(0);
        }

        headMode = HEAD_ANIM_DISABLED;
        headTarget = HEAD_POSE(HEAD_DIR_BACK, HEAD_TILT_LOWER);
        animState = scratchArr[0];
        waitHeadTurn();
        gameTick(8);

        animState = STATE_STAND_FACING_SCREEN;
        spriteLayer[SPRITE_ORGAN_PROP] = SPRITE_HIDDEN;
        layoutSlots();
        gameTick(0);

        if (songBuf != (char *) 0) {
                Mfree(songBuf);
                songBuf = (char *) 0;
        }
        organPlaying = NO;
}
