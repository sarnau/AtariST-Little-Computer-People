/* globals.h -- extern declarations for globals.c. */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "structs.h"

extern short bj_key;
extern char psg_ovol;
extern unsigned short g_wkadj;
extern unsigned short ani_cnt;
extern short g_secs;
extern short t_min;
extern short t_hour;
extern short date_day;
extern short dt_mon;
extern short dt_year;
extern PLAYER lcp;
extern BOOL16 ph_ans;
extern BOOL16 ph_call;
extern BOOL16 introSeq;
extern BOOL16 lunT_trg;
extern BOOL16 dinT_trg;
extern BOOL16 wkT_trg;
extern BOOL16 bedT_trg;
extern BOOL16 in_evrt;
extern short lastAct;
extern short g_trac;
extern short lcp_x;
extern short lcp_y;
extern BOOL16 g_lcldd;
extern long cprot_r;
extern short g_spdc;
extern BOOL16 alarm_p;
extern short lcp_watr;
extern short g_aliss;
extern short g_aqueu[];
extern short g_apriq[];
extern short g_hatas;
extern short g_hacur;
extern short g_hamod;
extern short g_hsfra;
extern long g_sfret;
extern BOOL16 g_actif;
extern BOOL16 dg_petok;
extern short g_wtx;
extern short g_wty;
extern short pst_arr[];
extern short lcp_frdO;
extern short studyDrO;
extern short lcp_clsO;
extern short lcp_cabO;
extern short lcp_drsO;
extern short lcp_toiO;
extern short lcp_flcO;
extern short lcp_bwlS;
extern short lcp_food;
extern short lcp_recP;
extern short lcp_tv;
extern short g_obisa[];
extern char mi_play;
extern short dg_bwlch;
extern short g_sfplf;
extern short g_sfpli;
extern BOOL16 g_rbact;
extern char* mi_sbuf;
extern short sng_cnt;
extern short org_cnt;
extern char scn_cmn[];
extern char *pex_name;
extern BOOL16 fire_act;
extern short fire_dur;
extern BOOL16 fire_ext;
extern short no_keyin;
extern short tx_sctm;
extern short g_srsdc;
extern short g_cdibp;
extern char* g_lttx;
extern char* g_ltlp[];
extern char* g_ltg[];
extern char* mo_names[];
extern short g_ltcwt[];
extern char g_ltscb[];
extern char in_str[];
extern short scn_dic[];
extern unsigned char comp_tok[];
extern short scn_siz;
extern char *scn_buf;
extern void scn_dec();
extern short* sv_bodyP;
extern short* sv_headP;
extern short vdihnd;
extern short vdi_hnd;
extern short gr_hwchar;
extern short gr_hhchar;
extern short gr_hwbox;
extern short gr_hhbox;
extern short vdi_colt[];
extern short contrl[];
extern short intin[];
extern short ptsin[];
extern short intout[];
extern short ptsout[];
extern void* g_dscp;
extern short main_pal[];
extern short g_clcop[];
extern short g_clcos[];
extern short skin_pal[];
extern char   mi_varR;
extern char  g_mspha;           /* STX: byte (moveb/tstb) */
extern unsigned char* mi_dbase;
extern unsigned char* mi_sqpos;
extern long g_msmap;
extern long mi_env;
extern char  mi_vel;            /* STX: byte */
extern char mi_dvel;
extern char  psg_cvol;          /* STX: byte */
extern char psg_dvol;
extern short g_mchcn;
extern short g_mtspb;
extern short mi_temp;
extern short aes_intO[];
extern long g_mtcou;
extern short mi_dwrm;
extern short g_mtdiv;
extern short g_mtpre;
extern long  mi_nxTk;           /* STX: long tick counters */
extern long  mi_lpTk;
extern char  g_msmsa;           /* STX: byte flag */
extern short mi_rlock;
extern long mi_svtv;
extern unsigned char* mi_seqE;
extern unsigned char* mi_dptr;
extern char mi_evTf;
extern char mi_nnOn;
extern char mi_lasT;
extern char mi_nnOf;
extern char mi_ccha;
extern char mi_cnot;
extern char mi_nmof;
extern char mi_nlpA;
extern short mi_nlp0;
extern char   mi_slop;          /* STX: byte flag */
extern short mi_ndt[];
extern short mi_evq[];
extern short mi_evi;
extern long mi_lstk[];
extern short mi_evcn;
extern short psg_rdel[];
extern short psg_racc[];
extern short mi_evrt[];
extern short mi_evtt[];
extern short mi_evrl[];
extern short mi_evst[];
extern unsigned char psg_rot[];
extern unsigned char mi_chmap[];
extern unsigned char mi_noSt[];
extern char  g_mcpro[];         /* STX: byte array */
extern char *mi_pgmap;          /* STX: a byte pointer, not an array */
extern unsigned char g_mstr[];
extern unsigned char g_msmk[];
extern BOOL16 g_moen;
extern unsigned char g_meve[];
extern long g_momap;
extern BOOL16 psg_out;
extern char   psg_ntAc;         /* STX: byte flag */
extern unsigned char psg_chNt[];
extern PSG_ENVELOPE psg_envelope[];
extern char g_mnlol;
extern char g_mnhil;
extern char g_sfcup;
extern short g_sfddh;
extern short g_sfddl;
extern long g_sfHz2;
extern unsigned char* mi_ntLp[];
extern char g_sfDoB[];
extern void* g_srlgb;
extern void* sv_lgb;
extern void* g_srptr;
extern short dsb_stor[];
extern short scr_scal;
extern short work_in[];
extern short wk_out[];
extern MFDB MFDB_A;
extern unsigned char scrbufA[];
extern unsigned char scrbufB[];
extern short g_cmmin;
extern short g_chhou;
extern BOOL16 g_sfacf;
extern short g_sfcur;
extern short g_sfdur;
extern short g_sfdos;
extern short g_sfdoc;
extern char  sf_pri[];          /* STX: 26-byte table (moveb + extw) */
extern unsigned char obj_file[];
extern unsigned char spr_file[];
extern MFDB g_obtmt[];
extern short g_obtaw[];
extern short g_obtah[];
extern short g_ltlic;
extern short g_ltpac;
extern unsigned short rec_ledt[];
extern short g_cmmip[];
extern short g_chhop[];
extern BOOL16 g_inpmd;
extern char g_cdinb[];
extern BOOL16 food_dlv;
extern short g_ptanf;
extern unsigned short last_hz;
extern long last_vbc;
extern void* sv_phb;
extern MFDB g_srmfd;
extern MFDB mf_scrp;
extern MFDB* cur_mf;
extern BOOL16 dg_vis;
extern short dg_idlcd;
extern BOOL16 dg_nrbwl;
extern BOOL16 g_deact;
extern short g_decou;
extern short dg_ltgtI;
extern short g_dgitx;
extern short g_dgiyo;
extern short g_dseat[];
extern short g_ddipt[];
extern short g_ddxot[];
extern short g_ddyot[];
extern char* cmd_inp;
extern short g_aprio;
extern MFDB g_semfi[];
extern MFDB g_semfm[];
extern short g_tp0xc[];
extern short g_tp0yc[];
extern short g_tp1xc[];
extern short g_tp1yc[];
extern short g_tp2xc[];
extern short g_tp2yc[];
extern short g_tp3xc[];
extern short g_tp3yc[];
extern short g_tpcoi[];
extern char g_ewb[];
extern char usr_buf[];
extern short mood_pri[];
extern char bm_lo[];
extern char* g_agwb;
extern char* g_wpdb;
extern short* crd_dat;
extern short g_wpci;
extern short g_agclc;
extern short g_aggun;
extern short g_agacu;
extern short ag_clue;
extern short g_agwol;
extern char g_aginb[];
extern char* g_agorw;
extern char g_agscw[];
extern char* g_agwgm[];
extern char* g_aggpr[];
extern BOOL16 mg_tofl;
extern short sv_vqta[];
extern short pk_round;
extern BOOL16 pk_quit;
extern short g_pcmon;
extern short g_ppmon;
extern short g_ppppa;
extern short g_pcbet;
extern short g_ppbet;
extern short pk_phase;
extern short pk_dsc[];
extern short g_pcdrp[];
extern short g_ppdrp[];
extern short pk_pwc[];
extern short pk_cwc[];
extern short g_pchc;
extern BOOL16 moff_f;
extern short pk_ch[];
extern short pk_ph[];
extern short pk_hrf[];
extern short pk_hsf[];
extern short pk_phrf[];
extern short pk_phsf[];
extern short pk_chrk;
extern short pk_phrk;
extern short pk_dslot;
extern short pk_sel[];
extern short pk_disc;
extern short pk_dpile[];
extern short pk_dpos;
extern short pk_phv;
extern short pk_bet;
extern BOOL16 pk_bluff;
extern BOOL16 pk_pass;
extern char *pk_bm;
extern char *pk_rm;
extern char *pk_tcm;
extern short pk_psh[];
extern short pk_pcc;
extern short pk_ccc;
extern short pk_pscc;
extern short pk_wpr;
extern BOOL16 pk_wrf;
extern BOOL16 pk_wcs;
extern BOOL16 pk_c1bj;
extern BOOL16 pk_c2bj;
extern BOOL16 pk_bs1;
extern BOOL16 pk_bs2;
extern short pk_cscore;
extern short pk_pscore;
extern char wp_ans[][12];
extern short wp_blk;
extern char* wp_prm[];
extern char* wp_succ[];
extern char* wp_fail[];
extern short crd_xa[];
extern short crd_ya[];
extern short crd_xb[];
extern short crd_yb[];
extern MFDB crd_mfdb[];
extern MFDB mf_scb_c;
extern BOOL16 g_dvdog;
extern BOOL16 ph_hu;
extern BOOL16 g_ptdoa;
/* The od_* frame ids are DATA globals (od_draw
   reads them from memory) but compile-time constants in the
   STX revision, which pushes the numbers as immediates.
   Nothing writes them at runtime, so a macro is exact. */
#define od_stcl    46
#define od_sto1    47
#define od_sto2    48
#define od_frcl    36
#define od_fro1    37
#define od_fro2    38
#define od_cbcl    19
#define od_cbo1    20
#define od_cbo2    21
#define od_med1    52
#define od_tocl    25
#define od_too1    26
#define od_too2    27
#define od_stof    42
#define od_fdcl    16
#define od_fdo1    17
#define od_fdo2    18
#define od_clcl    28
#define od_clo1    29
#define od_clo2    30
#define od_fir0    31
#define od_ficl    0
#define od_fio1    1
#define od_fio2    2
#define od_drcl    10
#define od_dro1    11
#define od_dro2    12
#define od_cbit    53

#endif /* GLOBALS_H */
