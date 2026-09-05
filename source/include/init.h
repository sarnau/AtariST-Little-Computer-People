/* init.h -- extern declarations for init.c. */

#ifndef INIT_H
#define INIT_H

extern void lcp_crnd();
extern void cl_drini();
extern void st_titl();
extern void drwCurs();
extern void inpNum();
extern void mq_intim();
extern void cntSong();
extern void initBRev();
extern void cs_mvIn();

/* st_titl's two helpers (0x718e and 0x72e6), used by st_titl
   itself and reached before their definitions in the unity unit. */
extern void stEnter();
extern void erChr();

#endif /* INIT_H */
