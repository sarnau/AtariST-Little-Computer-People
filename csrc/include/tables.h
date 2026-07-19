/* tables.h -- extern declarations for tables.c. */

#ifndef TABLES_H
#define TABLES_H

extern short g_rpxs[];
extern unsigned short rev_tab[];
extern unsigned short bm_msb_lsb[];
extern unsigned short bm_lsb_msb[];
extern short g_atact[];
extern short g_atmod[];
extern short g_atrel[];
extern short* sch_tab[];
extern short g_rphs[];
extern long bm32or[];
extern long bm32and[];

extern void initBM();

#endif /* TABLES_H */
