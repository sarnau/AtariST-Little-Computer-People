/* The last step of main, never returns.  If a saved game was loaded
   (loadedSave), the resident is placed at the study door and studyVisit
   brings him back into the house without saving.  When the copy
   protection failed (copyProtResult == 0) he only ever sleeps.  Otherwise
   the game speed is set and every frame is gameTick followed by
   chooseAction, which picks and runs the next action. */
void
gameLoop()
{
        /* 50 bytes of locals the body never reads.  They must stay:
           removing them changes the compiled code. */
        short   unused[25];

        if (loadedSave != 0) {
                posToXY(POS_TOP_STUDY_DOOR, &resX, &resY);
                resY -= 3;
                resX -= 10;
                studyVisit(NO, NO);
        }
        /* The sleep loop is the fall-through of the guard, and every
           loop is `while (1)`: `for (;;)` compiles differently. */
        if (copyProtResult == 0)
                while (1)
                        dozeOff(SLEEP_RANDOM);

        walkSpeed = 5;
        while (1) {
                gameTick(0);
                chooseAction();
        }
}
