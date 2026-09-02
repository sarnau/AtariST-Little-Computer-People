/* tables.h -- extern declarations for tables.c. */

#ifndef TABLES_H
#define TABLES_H

extern short g_rpxs[];
/* rev_tab is a plain (signed) short array, built at boot by initBRev
   from rv_msk/rv_val -- it is BSS, not initialised data. */
extern short rev_tab[];
extern short rv_msk[];
extern short rv_val[];
extern void rv_bld();
extern short g_atact[];
extern short g_atmod[];
extern short g_atrel[];
#ifdef FAITHFUL
extern short* sch_tab[];
#else
extern short sch_tab[][8];
#endif
extern short g_rphs[];
extern long bm32or[];
extern long bm32and[];


#endif /* TABLES_H */
