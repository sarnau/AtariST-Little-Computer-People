/* vdiown.h -- the game's own VDI bindings (see vdiown.c). */

#ifndef VDIOWN_H
#define VDIOWN_H

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
extern void     vro_cpy();

extern short *  vdipb[];

#endif /* VDIOWN_H */
