/* save.h -- extern declarations for save.c. */

#ifndef SAVE_H
#define SAVE_H

extern short fOpen();
extern void crFile();
extern short fr_read();         /* STX returns the Fread result */
extern void lcp_save();
extern short lc_load();
extern void lcp_std();

#endif /* SAVE_H */
