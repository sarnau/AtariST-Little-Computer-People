/* protos.h -- declarations of the program's functions.

   K&R declarations only (Alcyon takes no parameter lists), grouped by
   subsystem.  Every function returning something other than int or
   short must be declared before its first call, or Alcyon assumes int
   and the call compiles differently.  Data lives in globals.h,
   sprglobs.h and the table headers. */

#ifndef PROTOS_H
#define PROTOS_H

/* ---- Program start, boot and files ------------------------------------- */
extern void gameLoop();
extern int lcp_main();          /* see parts/main.c */
extern int main();
extern void rollResident();
extern void drawClock();
extern void titleScreen();
extern void hookTimerA();
extern void countSongs();
extern void initMirror();
extern void moveInScene();
/* titleScreen's two helpers: declared here because titleScreen reaches
   them before their definitions in the unity unit. */
extern void enterField();
extern void eraseChar();
extern short openFile();
extern void ensureFile();
extern short readFile();         /* returns the Fread result */
extern void saveFile();
extern short loadSavedGame();
extern void studyVisit();
/* Asset loaders.  File formats:
   OBJECTS, SPRITES: records of {h: BE16, w: BE16, then ceil(w/16)*4*2*h
     pixel bytes} -- 4 bitplanes interleaved per row, MSB first.  The
     loaders stop at the buffer end, at height 0 or after 64 records.
   BODY.LCP, PE2..PE6.LCP: {count: BE16, total_bytes: BE16, frames};
     each frame is a 16x21 image, 21 rows of 4 bitplane words (168
     bytes).  BODY.LCP holds BODY_FRAMES (98) frames, a PEx.LCP
     HEAD_FRAMES (66).  The masks are not in the files: buildMasks
     generates them at boot.  PEx is chosen by characterSpriteId
     (2..6).
   NAMES: fixed 10-byte records; rollResident seeks to a random one.
   .SCN: a nibble stream with a 15-word dictionary in bytes 2..31 of a
     32-byte header; nibble 0xF escapes to four more nibbles forming a
     literal word.  The payload starts at byte 32. */
extern void loadObjects();
extern void loadSprites();
extern short loadFrameFile();
extern void unpackFile();
extern void loadLetterText();
extern void outOfMemory();
extern void writeErrorAlert();
extern long checkCopyProt();

/* ---- The game tick, simulation and health ------------------------------ */
extern void gameTick();
extern void simStep();
extern void fallSick();
extern void startRecovery();
extern short rndRng();
/* rnd() is NOT declared here: see rnd.h. */

/* ---- Input and the command parser -------------------------------------- */
extern short getKey();
extern void handleKey();
extern short toUpper();
extern char* nextWord();
extern short lookupWord();
extern short matchCommand();

/* ---- The AI: picking and dispatching actions --------------------------- */
extern void runEvent();
extern void chooseAction();
extern void submitCommand();
extern short pickIdleAction();
extern void runAction();

/* ---- Resident actions (bodies in parts/) ------------------------------- */
extern void wakeFromAlarm();
extern void sayHello();
extern void yawnAndStretch();
extern void nodHead();
extern void petDog();
extern void callDog();
extern void idleShrug();
extern void peekAround();
extern void paceNervously();
extern void toggleTv();
extern void dozeOff();
extern void readNewspaper();
extern void getInOutOfBed();
extern void danceToMusic();
extern void drinkWater();
extern void useToilet();
extern void morningRoutine();
extern void nightRoutine();
extern void nodOk();
extern void tvStoop();
extern void recordStoop();
extern void playRecord();
extern void stopRecord();
extern void playOrgan();
extern void lightFire();
extern void sitWithDog();
extern void exercise();
extern void checkFrontDoor();
extern void tidyHouse();
extern void cleanUp();
extern void changeClothes();
extern void enterStudy();
extern void cookMeal();
extern void eatFromCabinet();
extern void feedDog();
extern void goToFridge();
extern void takeShower();
extern void brushTeeth();
extern void washHands();
extern void washAtSink();
extern void closeToiletDoor();
extern void closeBedCloset();
extern void putInFridge();
extern void closeFilingCab();
extern void openDresser();
extern void rummageCabinet();
extern void walkToFrontDoor();
extern void openFrontDoor();
extern void openKitchenCab();
extern void foodDelivery();
extern void bookDelivery();
extern void recordDelivery();
extern void dogFoodDelivery();
extern void answerPhone();
extern void typeChar();
extern short typeString();
extern void writeLetter();
extern void useComputer();
extern void playGame();

/* ---- Walking, positions and the dog ------------------------------------ */
extern short walkToTarget();
extern void nextWaypoint();
extern void dogNextWaypt();
extern void playFootstep();
extern void walkStep();
extern void posToXY();
extern short floorOfY();
extern short calcWeekday();
extern void placeDog();
extern void moveDog();
extern void setDogSprite();

/* ---- Drawing: primitives, house, sprites, head, clock, TV -------------- */
extern void drawLine();
extern void beginDraw();
extern void endDraw();
extern void paperRow();
extern void stripeRow();
extern void blackRow();
extern void panelBegin();
extern void panelEnd();
extern void drawPixel();
extern void copyBlocks32();
extern void copyScreen();
extern void initAes();
extern void vdiInit();
extern void vdiClear();          /* vdiInit's second half */
extern void initHouseBuf();
extern void textBig();
extern void textNormal();
extern void hideMouse();
extern void redrawHands();
extern void drawObject();
extern void fillPanel();
extern short tvOn();
extern short tvOff();
extern void drawFoodCab();
extern void updateWaterTank();
extern void renderFrame();
extern void pickClothes();
extern void pickSkin();
extern void setSkinColor();
extern void drawTvPicture();
extern void tvNoise();
extern void scrollStrip();
extern void printChar();
extern void animRecPlayer();
extern void printString();
extern void initMfdb();
extern void drawSlot();
extern void stepHead();
extern void drawHands();
extern void tvClearAnim();
extern void tvBounce();
extern void tvPattern();

/* ---- Sound effects, the MIDI/PSG sequencer and PSG I/O ----------------- */
extern void sfxSelect();
extern void stopSfx();
extern void sfxTvClick();
extern void sfxGreeting();
extern void sfxSpeech();
extern void sfxHeadNod();
extern void playDoorbell();
extern void typeKeySound();
extern void sfxClick();
extern void loadSounds();
extern void playSongFile();
extern void startSfx();
extern void startSong();
extern void parseSongHeader();
extern void resetPrograms();
extern unsigned char* skipTextField();
extern void initSongState();
extern void armSequencer();
extern void unpackChanMap();
extern void buildNoteMap();
extern void sendProgChange();
extern short sendMidiEvent();
extern void seqAdvance();
extern void stepEnvelopes();
extern void peekNoteDur();
extern void pushLoop();
extern unsigned char* popLoop();
extern short removeQueued();
extern void sendNoteOff();
extern void expireNotes();
extern void queueNote();
extern short parseEvents();
extern void stopSequencer();
extern void unhookTimerA();
/* timerAIsr lives in mq_tick.s (the Timer-A interrupt routine); it is
   declared so hookTimerA can pass its address to Xbtimer. */
extern void timerAIsr();
extern void aciaWrite();
extern void copyEnvelope();
extern void psgWrite();
extern void psgMixer();

/* ---- Minigames --------------------------------------------------------- */
extern void mgSetup();
extern void panelErase();
extern void leaveGameTable();
extern void rejoinTable();
extern short mgWaitKey();
extern void anaClrBottom();
extern void anaClrWord();
extern void anaClrIntro();
extern void anaClrGuess();
extern void anaDrawPrompt();
extern void anaShowWord();
extern void anaIntroText();
extern short anaStrMatch();
extern void anaPickWord();
extern void playAnagrams();
extern void playWordPuzzle();
extern void playWar();
extern void cardMessage();
extern void dispCompChips();
extern void dispPlyrChips();
extern void dispPot();
extern short popCard();
extern void pushCard();
extern void potToWinner();
extern short cardKeyInput();
extern void cardDraw();
extern void playPoker();
extern void playBlackjack();
extern void wpzMessage();
extern void wpzRender();
extern void wpzSolve();
extern void cardLoad();

#endif /* PROTOS_H */
