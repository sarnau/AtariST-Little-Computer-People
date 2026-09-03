/* vdiown.h -- the game's own VDI bindings (see vdiown.c). */

#ifndef VDIOWN_H
#define VDIOWN_H

/* LCP_STX has ONE trap dispatcher, the VDIBIND-shaped gsx1 that
   vdistx_a.s supplies.  vdi_go and vdi_go2 are older spellings the
   port's sources still use. */
#define vdi_go   gsx1
#define vdi_go2  gsx1
extern void     vdi_go();       /* vdi_go.s: trap #2 with vdipb */
extern void     vsl_color();
extern void     vst_color();
extern void     vsf_color();
extern void     vsf_interior();
extern void     vsf_style();
extern void     vswr_mode();
extern void     v_pline();
extern void     v_gtext();
extern void     v_bar();
extern void     vroCpyD();

extern short *  vdipb[];

#endif /* VDIOWN_H */
