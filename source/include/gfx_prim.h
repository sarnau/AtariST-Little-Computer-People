/* gfx_prim.h -- extern declarations for gfx_prim.c. */

#ifndef GFX_PRIM_H
#define GFX_PRIM_H

extern void drwLine();
extern void sc_sdtb();
extern void sc_sdtf();
extern void sc_firw();
extern void sc_firs();
extern void sc_firb();
extern void initVdi();
extern void exitVdi();
extern void drwPixel();
extern void blkcp32();
extern void cpyScr();
extern void aes_init();
extern void sc_ers();
extern void vdi_init();
extern void stpScrB();
extern void vst_h20();
extern void rst_vsth();
extern void vdi_cprt();
extern void moff();
extern void drwBar();

#endif /* GFX_PRIM_H */
