/* tables.h -- extern declarations for tables.c. */

#ifndef TABLES_H
#define TABLES_H

extern short g_rpxs[];
/* The STX build reads rev_tab as a plain (signed) short: its sp_flih
   loads the table without the clr.w zero-extension LCP_ORG's build
   emits. */
#ifdef FAITHFUL
extern unsigned short rev_tab[];
#else
extern short rev_tab[];
#endif
extern short g_atact[];
extern short g_atmod[];
extern short g_atrel[];
extern short* sch_tab[];
extern short g_rphs[];
extern long bm32or[];
extern long bm32and[];

extern void initBM();

#endif /* TABLES_H */
