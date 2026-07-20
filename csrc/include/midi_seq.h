/* midi_seq.h -- extern declarations for midi_seq.c. */

#ifndef MIDI_SEQ_H
#define MIDI_SEQ_H

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
extern void mq_spgm();
extern void mq_qnne();
extern short mq_pars();
extern void mq_stop();
extern void mq_extm();

#endif /* MIDI_SEQ_H */
