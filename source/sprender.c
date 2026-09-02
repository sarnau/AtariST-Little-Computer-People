/*
 * sprender.c -- masked-blit sprite renderer (two-pass NOTS_AND_D + S_XOR_D).
 * addr: sp_draw(), sp_iniM()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <vdibind.h>
#include <obdefs.h>
#include "globals.h"
#include "sprender.h"
#include "vdiown.h"
#include "sprglobs.h"

/* sp_iniM -> parts/sp_iniM.c (STX: 0x6612, right after stpScrB). */
#ifdef FAITHFUL
#include "parts/sp_iniM.c"
#endif

/* addr: sp_draw() */
void
sp_draw(index)
short   index;
{
        short   x1;
        short   y1;
        short   w;
        short   h;

        x1 = g_sepex[index];
        y1 = g_sepey[index];
        w  = g_seacw[index];
        h  = g_seach[index];

        sp_iniM(0L, &g_semfi[index],
                         g_seaim[index], w, h);
        sp_iniM(0L, &g_semfm[index],
                         g_seams[index],  w, h);

        vroCpyD(vdihnd, NOTS_AND_D,
                index * 20 + (long) g_semfm, (long) &g_srmfd,
                0, 0, w - 1, h - 1,
                x1, y1, x1 + w - 1, y1 + h - 1);
        vroCpyD(vdihnd, S_XOR_D,
                index * 20 + (long) g_semfi, (long) &g_srmfd,
                0, 0, w - 1, h - 1,
                x1, y1, x1 + w - 1, y1 + h - 1);
}
