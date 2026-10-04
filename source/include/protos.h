/* protos.h -- declarations of the program's functions.

   K&R declarations only (Alcyon takes no parameter lists), grouped by
   subsystem.  Every function returning something other than int or
   short must be declared before its first call, or Alcyon assumes int
   and the call compiles differently.  Data lives in globals.h,
   sprglobs.h and the table headers. */

#ifndef PROTOS_H
#define PROTOS_H

/* ---- Program start, boot and files ------------------------------------- */
extern void gameLoop();
extern int lcp_main();          /* see parts/main.c */
extern int main();
extern void lcp_crnd();
extern void cl_drini();
extern void st_titl();
extern void drwCurs();
extern void inpNum();
extern void mq_intim();
extern void cntSong();
extern void initBRev();
extern void cs_mvIn();
extern void stEnter();
extern void erChr();
extern short fOpen();
extern void crFile();
extern short fr_read();         /* returns the Fread result */
extern void lcp_save();
extern short lc_load();
extern void lcp_std();
extern void ldObj();
extern void ldSpr();
extern short al_loal();
extern void fr_reac();
extern void fl_ltpl();
extern void er_nomem();
extern void er_write();
extern long cp_main();

/* ---- The game tick, simulation and health ------------------------------ */
extern void gameTick();
extern void gameSim1();
extern void lcp_sick();
extern void lcp_rcov();
extern short rndRng();
/* rnd() is NOT declared here: see rnd.h. */

/* ---- Input and the command parser -------------------------------------- */
extern short getKey();
extern void deal_kc();
extern short lcp_upp();
extern char* cmd_upp();
extern short chk_vwd();
extern short chk_encm();

/* ---- The AI: picking and dispatching actions --------------------------- */
extern void execEv();
extern void chk_actT();
extern void prsCmd();
extern short chk_timA();
extern void doAct();

/* ---- Resident actions (bodies in parts/) ------------------------------- */
extern void a_wakfa();
extern void a_hello();
extern void a_yawas();
extern void a_nodh();
extern void a_petd();
extern void a_calld();
extern void a_wandi();
extern void a_peeka();
extern void a_pacen();
extern void a_toggt();
extern void a_sleep();
extern void a_readn();
extern void a_gioob();
extern void a_dance();
extern void a_drink();
extern void a_uset();
extern void a_wakum();
extern void a_gotbn();
extern void a_nodok();
extern void li_lool();
extern void li_loor();
extern void a_lists();
extern void a_playp();
extern void a_plawr();
extern void a_lighf();
extern void a_socwd();
extern void a_sitae();
extern void a_chefd();
extern void a_tidyh();
extern void a_cleau();
extern void a_opcbc();
extern void a_opcuc();
extern void a_eatm();
extern void a_kitcc();
extern void a_feedd();
extern void a_gesff();
extern void a_takes();
extern void a_brust();
extern void a_washh();
extern void a_driwa();
extern void a_clotd();
extern void a_clocd();
extern void a_opecf();
extern void a_opcfc();
extern void a_opecd();
extern void a_watat();
extern void wkFrDr();
extern void a_opcfd();
extern void a_opecc();
extern void er_food();
extern void er_bood();
extern void er_recd();
extern void er_dogf();
extern void ev_ansPh();
extern void lt_tyca();
extern short lt_tysa();
extern void a_writl();
extern void a_playc();
extern void a_plaag();

/* ---- Walking, positions and the dog ------------------------------------ */
extern short lcp_wkD();
extern void lcp_flwp();
extern void dg_wkPth();
extern void lcp_fstp();
extern void lcp_path();
extern void hs_posXY();
extern short getFlrY();
extern short cWkday();
extern void dg_ipos();
extern void dg_mvAni();
extern void sp_spud();

/* ---- Drawing: primitives, house, sprites, head, clock, TV -------------- */
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
extern void vdi_init();
extern void vdi_cls();          /* vdi_init's second half */
extern void stpScrB();
extern void vst_h20();
extern void rst_vsth();
extern void moff();
extern void cl_redrH();
extern void od_draw();
extern void fillTopR();
extern short tt_on();
extern short tt_off();
extern void sc_drfc();
extern void updWtLv();
extern void sc_ren8();
extern void pa_cloc();
extern void pa_skic();
extern void lcp_upal();
extern void td_line();
extern void td_nois();
extern void sc_sctd();
extern void prCh();
extern void rp_anim();
extern void strPr();
extern void sp_iniM();
extern void sp_draw();
extern void sp_lcha();
extern void cl_drwH();
extern void tv_scrc();
extern void tv_boul();
extern void tv_patl();

/* ---- Sound effects, the MIDI/PSG sequencer and PSG I/O ----------------- */
extern void sf_sele();
extern void sf_so();
extern void p_sftvc();
extern void p_sfgrt();
extern void p_sfspe();
extern void p_sfhnd();
extern void p_dobls();
extern void lt_sets();
extern void sfClick();
extern void sf_sl();
extern void sgPlay();
extern void sf_irqp();
extern void mq_inis();
extern void mq_parh();
extern void mq_resp();
extern unsigned char* mq_skip();
extern void mq_setp();
extern void mq_stap();
extern void mq_pacm();
extern void mq_bust();
extern void mq_sepc();
extern short mq_dise();
extern void mq_advs();
extern void psg_upEn();
extern void mq_rdur();
extern void mq_pshl();
extern unsigned char* mq_popl();
extern short mq_rmev();
extern void mq_snof();
extern void mq_expN();
extern void mq_qnne();
extern short mq_pars();
extern void mq_stop();
extern void mq_extm();
extern void mq_tick();
extern void mowrit();
extern void psg_cpE();
extern void psg_wr();
extern void psg_mix();

/* ---- Minigames --------------------------------------------------------- */
extern void mg_stp();
extern void plEr();
extern void lcp_lgt();
extern void lcp_rgt();
extern short mg_wkev();
extern void ag_csb();
extern void ag_cwda();
extern void ag_cswa();
extern void ag_cgpa();
extern void ag_sgp();
extern void ag_dwl();
extern void ag_intr();
extern short ag_matc();
extern void ag_ssw();
extern void ag_main();
extern void wp_main();
extern void pk_wrMn();
extern void pk_pmsg();
extern void pk_awp();
extern void pk_dppm();
extern void pk_dpot();
extern short pk_rmch();
extern void pk_actd();
extern void pk_annr();
extern short pk_inph();
extern void pk_drcs();
extern void pk_main();
extern void pk_bjMn();
extern void wp_shwm();
extern void wp_rtmp();
extern void wp_solv();
extern void pk_ldCrd();

#endif /* PROTOS_H */
