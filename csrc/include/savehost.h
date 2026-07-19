/* savehost.h -- extern declarations for savehost.c. */

#ifndef SAVEHOST_H
#define SAVEHOST_H

extern short Fopen();
extern short Fcreate();
extern long Fread();
extern long Fwrite();
extern short Fclose();
extern void* Malloc();
extern long Mfree();
extern void* Fgetdta();
extern short Fsfirst();
extern short Fsnext();
extern short Cconis();
extern long Crawcin();
extern short Dsetpath();

#endif /* SAVEHOST_H */
