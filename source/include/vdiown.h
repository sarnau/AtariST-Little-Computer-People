/* vdiown.h -- the game's own VDI bindings (see vdiown.c). */

#ifndef VDIOWN_H
#define VDIOWN_H

/* There is ONE trap dispatcher, the VDIBIND-shaped gsx1 that
   vdistx_a.s supplies.  vdi_go and vdi_go2 are alternative spellings
   still used by the sources. */
#define vdi_go   gsx1
#define vdi_go2  gsx1
extern void     vdi_go();       /* trap #2 with vdipb */
extern void     vsl_color();
extern void     vst_color();
extern void     vsf_color();
extern void     vsf_interior();
extern void     vsf_style();
extern void     vswr_mode();
extern void     v_pline();
extern void     v_gtext();
extern void     v_bar();
extern void     blitRect();

extern short *  vdipb[];

/* VDI function numbers -- what each binding writes into contrl[0]
   (the GEM VDI opcode; contrl[1] is the ptsin count, contrl[3] the
   intin count).  Only the ones this program issues. */
#define VDI_V_PLINE             6
#define VDI_V_GTEXT             8
#define VDI_V_GDP               11      /* sub-function in contrl[5] */
#define VDI_VST_HEIGHT          12
#define VDI_VSL_COLOR           17
#define VDI_VST_COLOR           22
#define VDI_VSF_INTERIOR        23
#define VDI_VSF_STYLE           24
#define VDI_VSF_COLOR           25
#define VDI_VSWR_MODE           32
#define VDI_VQT_ATTRIBUTES      38
#define VDI_V_OPNVWK            100
#define VDI_VRO_CPYFM           109

/* v_gdp sub-functions (contrl[5]). */
#define GDP_BAR                 1

#endif /* VDIOWN_H */
