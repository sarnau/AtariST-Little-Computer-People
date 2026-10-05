/* tables.h -- extern declarations for tables.c. */

#ifndef TABLES_H
#define TABLES_H

extern short posXHalf[];
/* mirrorTable is a plain (signed) short array, built at boot by initMirror
   from mirrorSrcBit/mirrorDstBit -- it is BSS, not initialised data. */
extern short mirrorTable[];
extern short mirrorSrcBit[];
extern short mirrorDstBit[];
extern void buildMirrorTable();
extern short activeActions[];
extern short moderateActions[];
extern short relaxedActions[];
extern short scheduleTiers[][8];
extern short posYOffset[];
extern long bitSet32[];
extern long bitClear32[];


#endif /* TABLES_H */
