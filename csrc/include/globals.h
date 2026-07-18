/*
 * globals.h -- extern declarations for global game state.
 *
 * Definitions live in globals.c.  This header exposes only the globals
 * currently referenced by ported modules; new externs get added as
 * additional subsystems come online.  Names preserved from Ghidra so
 * decompiled-source cross-reference works one-to-one.
 *
 * addr: individual globals by their Ghidra symbol names.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "structs.h"

/* ---- Timing ------------------------------------------------------------ */
extern short    ani_cnt;         /* 8Hz frame counter */
extern short    g_secs;           /* 0..59 game-seconds  */

/* ---- Clock ------------------------------------------------------------- */
extern short    t_min;
extern short    t_hour;
extern short    date_day;
extern short    dt_mon;
extern short    dt_year;

/* ---- Character --------------------------------------------------------- */
extern PLAYER   lcp;                            /* the resident LCP */

/* ---- Event queue / flags ---------------------------------------------- */
extern BOOL16   ph_ans;
extern BOOL16   ph_call;
extern BOOL16   introSeq;

/* ---- Once-per-day action triggers (cleared by daily_rs) */
extern BOOL16   lunT_trg;
extern BOOL16   dinT_trg;
extern BOOL16   wkT_trg;
extern BOOL16   bedT_trg;

/* ---- Calendar table (defined in calendar.c) --------------------------- */
extern short    days_pmo[];

/* ---- Deferred event queue (defined in events.c) ----------------------- */
extern short    g_trel[];
extern BOOL16   in_evrt;

/* ---- Command / AI state ----------------------------------------------- */
extern short    lastAct;
extern short    g_trac;

/* ---- LCP position and world state ------------------------------------- */
extern short    lcp_x;
extern short    lcp_y;
extern short    g_lcldd;
extern short    cprot_r;
extern short    g_spdc;

/* ---- Alarm / water state --------------------------------------------- */
extern BOOL16   alarm_p;
extern short    lcp_watr;

/* ---- Command queue (populated by keyboard input) --------------------- */
extern short    g_aliss;
extern short    g_aqueu[];
extern short    g_apriq[];

/* ---- Head animation / sound cross-cutting ---------------------------- */
extern short    g_hatas;
extern short    g_hacur;
extern short    g_hamod;
extern short    g_hsfra;
extern long     g_sfret;
extern BOOL16   g_actif;
extern BOOL16   dg_petok;
extern short    g_wtx;
extern short    g_wty;
extern short    pst_arr[];

/* ---- Time-of-day globals used by chk_actT ------- */
/* (these are aliased into the PLAYER struct: lcp.lunch_hour etc.
   Nothing to declare here.) */

/* ---- Cross-file helpers ---------------------------------------------- */
extern void     lcp_hwt();
extern void     gameTick();
extern void     a_getd();

/* ---- Door / furniture runtime unpacked flags (mirror of the packed
   bit-field in lcp.door_states_and_flags; unpacked at load time and
   repacked at save time). --------------------------------------------- */
extern short    lcp_frdO;
extern short    studyDrO;
extern short    lcp_clsO;
extern short    lcp_cabO;
extern short    lcp_drsO;
extern short    lcp_toiO;
extern short    lcp_flcO;
extern short    lcp_bwlS;
extern short    lcp_food;
extern short    lcp_recP;
extern short    lcp_tv;

/* ---- Object IDs for door art (indexes into _object_images[]) --------- */
extern short    g_obids;
extern short    g_obi07;
extern short    g_obi08;
extern short    g_obidf;
extern short    g_obi05;
extern short    g_obi06;
extern short    g_obicc;
extern short    g_obico;
extern short    g_obi02;
extern short    g_obipc;
extern short    g_obidt;
extern short    g_obi09;
extern short    g_obi10;
extern short    g_obiso;
extern short    g_obisa[];
extern short    g_obi15;
extern short    g_obi16;
extern short    g_obi17;

extern BOOL16   mi_play;
extern short    dg_bwlch;
extern short    g_sfplf;
extern short    g_sfpli;

/* ---- Leisure / music / fire globals ---------------------------------- */
extern BOOL16   g_rbact;
extern char *   mi_sbuf;
extern short    org_cnt;
extern BOOL16   fire_act;
extern short    fire_dur;
extern BOOL16   fire_ext;
extern short    no_keyin;
extern short    tx_sctm;
extern short    g_srsdc;
extern short    g_cdibp;

/* ---- Letter subsystem ------------------------------------------------- */
extern char *   g_lttx;
extern char *   g_ltlp[];
extern char *   g_ltg[];
extern char *   mo_names[];
extern short    g_ltcwt[];
extern char     g_ltscb[];
extern unsigned char comp_tok[];

/* ---- Object IDs for closet + fireplace art --------------------------- */
extern short    g_obidc;
extern short    g_obi03;
extern short    g_obi04;
extern short    g_obifo;
extern short    g_obifa[];
extern short    g_obifc;
extern short    g_obi13;
extern short    g_obi14;
extern short    g_obi11;
extern short    g_obido;
extern short    g_obi12;
extern short    g_obibg;

/* ---- Saved LCP body/head pointers for hide/show ---------------------- */
extern short *  sv_bodyP;
extern short *  sv_headP;

/* ---- VDI handle + color table (populated at graphics init) ----------- */
extern short    vdihnd;
extern short    vdi_colt[];

/* ---- VDI parameter block ------------------------------------------- */
/* Shared per-call scratch arrays used by the VDI wrappers.  Defined
   by the linked Alcyon gemlib (vdibind.a). */
extern short    contrl[];
extern short    intin[];
extern short    ptsin[];
extern short    intout[];
extern short    ptsout[];

/* ---- Screen buffer pointers ------------------------------------------ */
extern void *   g_dscp;

/* ---- Palette state --------------------------------------------------- */
extern short    main_pal[];
extern short    g_clcop[];
extern short    g_clcos[];
extern short    skin_pal[];

/* ---- MIDI sequencer state -------------------------------------------- */
extern BOOL16   g_molof;
extern BOOL16   mi_varR;
extern short    g_mspha;
extern unsigned char *  mi_dbase;

/* ---- MIDI sequencer state ------------------------------------------- */
extern unsigned char *  mi_sqpos;
extern long             g_msmap;
extern long             mi_env;
extern short            mi_vel;
extern short            mi_dvel;
extern short            psg_cvol;
extern short            psg_dvol;
extern short            g_mnevi;
extern short            g_mnevc;
extern short            g_mtspb;
extern short            mi_temp;
extern short            aes_intO[];

extern long             g_mtcou;
extern short            mi_dwrm;
extern short            g_mtdiv;
extern short            g_mtpre;
extern short            g_medu;
extern short            mi_nxTk;
extern short            mi_lpTk;
extern BOOL16           g_msmsa;

extern unsigned char    mi_chmap[];
extern short            g_mcpro[];
extern short            mi_pgmap[];
extern unsigned char    g_mstr[];
extern unsigned char    g_msmk[];
extern BOOL16           g_moen;
extern unsigned char    g_meve[];
extern long             g_momap;

/* ---- PSG channel state ---------------------------------------------- */
extern BOOL16           psg_out;
extern BOOL16           psg_ntAc;
extern unsigned char    psg_chNt[];
extern PSG_ENVELOPE     psg_envelope[];
extern unsigned short   psg_freq[];

extern short            env_val;                   /* transpose base */
extern char             g_mnlol;
extern char             g_mnhil;
extern short            g_mccha;

/* ---- SFX / Dosound state -------------------------------------------- */
extern short            g_sfcup;
extern short            g_sfddh;
extern short            g_sfddl;
extern long             g_sfHz2;
extern unsigned char *  mi_ntLp[];
extern char             g_sfDoB[];

/* ---- Screen buffer state -------------------------------------------- */
extern void *   g_srlgb;
extern void *   sv_lgb;
extern void *   g_srptr;
extern short *  g_dsb;

/* ---- Clock display ---------------------------------------------------- */
extern short    g_cmmin;
extern short    g_chhou;

/* ---- Sound-effect queue --------------------------------------------- */
extern BOOL16   g_sfacf;
extern short    g_sfcur;
extern short    g_sfdur;
extern short    g_sfdos;
extern short    g_sfdoc;
extern short    sf_pri[];

/* ---- Object / sprite backing storage (populated by asset loaders) --- */
extern unsigned char    obj_file[];
extern unsigned char    spr_file[];
extern MFDB     g_obtmt[];
extern MFDB     g_setmt[];
extern short    g_obtaw[];
extern short    g_obtah[];
extern short    g_setaw[];
extern short    g_setah[];

/* Legacy pointer form retained for od_draw / render.c callers that
   walk the mfdb table by index arithmetic. */
extern void *   g_otmfd;
/* mf_scrp now declared with the frame-timing MFDBs below. */

/* ---- Record player / letter needle state (packed inside letter subsys) */
extern short    g_ltlic;
extern short    g_ltpac;
extern unsigned short   rec_ledt[];

/* ---- Clock hand endpoint tables ------------------------------------- */
extern short    g_cmmip[];
extern short    g_chhop[];

/* ---- Keyboard / command input --------------------------------------- */
extern BOOL16   g_inpmd;
extern char     g_cdinb[];
extern BOOL16   food_dlv;
extern short    g_ptanf;

/* ---- Frame-timing counters ------------------------------------------ */
extern short    last_hz;
extern long     last_vbc;
extern void *   sv_phb;

/* ---- Screen MFDB descriptors ---------------------------------------- */
extern MFDB     g_srmfd;
extern MFDB     mf_scrp;        /* alias with older name */
extern MFDB *   cur_mf;

/* ---- 200 Hz + VBL clock (host-side we roll these ourselves) --------- */
extern short    g_hzhi;
extern short    g_hzlo;
extern long     _vbclock;

/* ---- Dog wander behaviour ------------------------------------------- */
extern BOOL16   dg_vis;
extern short    dg_idlcd;
extern BOOL16   dg_nrbwl;
extern BOOL16   g_deact;
extern short    g_decou;
extern short    dg_ltgtI;
extern short    g_dseat[];
extern short    g_ddipt[];
extern short    g_ddxot[];
extern short    g_ddyot[];

/* ---- Command parser state ------------------------------------------- */
extern char *   cmd_inp;
extern short    g_aprio;

/* ---- Sprite MFDB arrays (one per hardware slot) --------------------- */
extern MFDB     g_semfi[];
extern MFDB     g_semfm[];

/* ---- TV animation coord tables ------------------------------------- */
extern short    g_tp0xc[];
extern short    g_tp0yc[];
extern short    g_tp1xc[];
extern short    g_tp1yc[];
extern short    g_tp2xc[];
extern short    g_tp2yc[];
extern short    g_tp3xc[];
extern short    g_tp3yc[];
extern short    g_tpcoi[];

/* ---- NLP parser tables (populated at runtime from vocabulary data)  */
extern unsigned char    g_ewb[];
extern char             usr_buf[];
extern short            mood_pri[];
extern char *           vwd_tab[];
extern short            ew2pos[];
extern short            g_ew2b[];
extern unsigned char    bm_lo[];
extern WORD_TO_ACTION   g_ew2a[];

/* ---- Mini-game state -------------------------------------------------- */
extern char *   g_agwb;
extern char *   g_wpdb;
extern short *  crd_dat;

extern short    g_wpci;
extern short    g_agclc;
extern short    g_aggun;
extern short    g_agacu;
extern short    ag_clue;
extern short    g_agwol;
extern char     g_aginb[];
extern char *   g_agorw;
extern char     g_agscw[];
extern char *   g_agwgm[];
extern char *   g_aggpr[];
extern BOOL16   mg_tofl;
extern short    sv_vqta[];

extern short    pk_round;
extern BOOL16   pk_quit;
extern short    g_pcmon;
extern short    g_ppmon;
extern short    g_ppppa;
extern short    g_pcbet;
extern short    g_ppbet;
extern short    pk_phase;
extern short    pk_dsc[];
extern short    g_pcdrp[];
extern short    g_ppdrp[];
extern short    pk_pwc[];
extern short    pk_cwc[];
extern short    g_pchc;
extern BOOL16   moff_f;
extern short    crd_xa[];
extern short    crd_ya[];
extern short    crd_xb[];
extern short    crd_yb[];
extern short    pk_ch[];
extern short    pk_ph[];
extern short    pk_hrf[];
extern short    pk_hsf[];
extern short    pk_phrf[];
extern short    pk_phsf[];
extern short    pk_chrk;
extern short    pk_phrk;
extern short    pk_dslot;
extern short    pk_sel[];
extern short    pk_disc;
extern short    pk_dpile[];
extern short    pk_dpos;
extern short    pk_phv;
extern short    pk_bet;
extern BOOL16   pk_bluff;
extern BOOL16   pk_pass;
extern char     pk_bm[];
extern char     pk_rm[];
extern char     pk_tcm[];
extern short    pk_psh[];
extern short    pk_pcc;
extern short    pk_ccc;
extern short    pk_pscc;
extern short    pk_wpr;
extern BOOL16   pk_wrf;
extern BOOL16   pk_wcs;
extern BOOL16   pk_c1bj;
extern BOOL16   pk_c2bj;
extern BOOL16   pk_bs1;
extern BOOL16   pk_bs2;
extern short    pk_cscore;
extern short    pk_pscore;

/* Card graphics: 54 MFDB descriptors covering 52 card faces + 1 shared
   back + 1 highlight overlay pattern, all sharing crd_dat as their
   pixel storage.  mf_scb_c is a screen-buffer MFDB
   sized to the mini-game display area (320x77). */
extern MFDB     crd_mfdb[];
extern MFDB     mf_scb_c;

/* ---- Delivery / phone / petting flags -------------------------------- */
extern BOOL16   g_dvdog;
extern BOOL16   ph_hu;
extern BOOL16   g_ptdoa;

/* ---- Sprite head pipeline (defined in sprglobs.c) --------------- */
extern short    g_hsbuf[];
extern short    g_hsmas[];
extern short    g_hsmif;
extern short *  pex_ptr;                   /* source head sheet */
extern short *  hd_shp;                /* source head masks */
extern short    mood_hfo[];
extern short    hd_xoff[];
extern short    hd_hgt[];
extern short    hd_dang[];
extern short    hd_mvd[];
extern short    hd_tilt[];
extern short    g_hadec;

/* ---- Bit-reverse LUT used by sp_lcpf / sp_flih - */
extern unsigned short   rev_tab[];

/* ---- Walk-pathfinding state ------------------------------------------ */
extern short    g_wyx;
extern short    g_wyy;
extern short    lcp_stR;
extern BOOL16   fs_trg;
extern short    g_hastl;
extern short    stair_ty;
extern short    stair_by;

/* ---- Utility functions (implemented in movement.c etc) ---------------- */
extern void     hs_posXY();
extern short    getFlrY();
extern short    cWkday();

/* ---- LCP animation state (defined in globals.c) ----------------------- */
extern short    lcp_st;
extern short    lcp_face;
extern short    g_lcyof;
extern short    g_lcieo;
extern short    g_lssh;
extern short    dbg_hide;

/* ---- Dog state -------------------------------------------------------- */
extern short    dog_x;
extern short    dog_y;
extern short    g_dtx;
extern short    g_dty;
extern short    g_dyx;
extern short    g_dyy;
extern short    g_dwanc;
extern short    g_dsid;
extern short    dg_stair;
extern short    dg_init;

/* ---- Hardware sprite double-buffer (8 slots) -------------------------- */
extern short    g_sepef[];
extern short *  g_sepim[];
extern short *  g_sepms[];
extern short    g_sepex[];
extern short    g_sepey[];
extern short    g_sepeh[];
extern short    g_sepew[];
extern short *  g_seaim[];
extern short *  g_seams[];
extern short    g_seacx[];
extern short    g_seacy[];
extern short    g_seach[];
extern short    g_seacw[];

/* ---- Sprite definition arrays (indexed by SPRITE_ID, 60 slots) -------- */
extern short *  g_sedim[];
extern short *  g_sedms[];
extern short    g_sedeh[];
extern short    g_sedew[];
extern short    g_selaf[];
extern short    g_seslm[];

/* ---- Body / carry frame tables (indexed by PLAYER_STATE) -------------- */
extern short    body_frT[];
extern short    cy_frT[];
extern short    body_yof[];

/* ---- LCP body / head buffers and file pointers ------------------------ */
extern short *  body_ptr;
extern short *  body_shp;
extern short    g_lsimg[];
extern short    g_lsmas[];

/* ---- Dog sprite tables ----------------------------------------------- */
extern short    g_dwanf[];
/* PTR_ARRAY_0005a156/0x54016 are the shared sprite_def_image/sprite_def_mask
   tables declared as g_sedim/g_sedms elsewhere in this header. */
extern short    g_dfimb[];
extern short    g_dfmab[];

/* ---- Floor geometry (used by pathfinding) ---------------------------- */
extern short    flr_by[];
extern short    flr_cy[];
extern short    stair_wp[];

/* ---- Tick-loop counters ----------------------------------------------- */
extern short    subAniC;
extern short    ani_cnt;

/* ---- Externals implemented in other TUs (subset used by sim.c) -------- */
extern short    rndRng();                  /* random.c */
extern void     lcp_sick();              /* health.c  */
extern void     lcp_upal();    /* render.c  */
extern void     daily_rs();     /* ai.c      */
extern short    daysInMo();                /* calendar.c*/
extern void     putEv();            /* ai.c      */
extern short    getEv();          /* events.c  */
extern void     execEv();                /* ai.c      */
extern void     chk_actT();/* ai.c      */
extern void     doAct();                    /* actions.c */

#endif  /* GLOBALS_H */
