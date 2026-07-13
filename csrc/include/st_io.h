/*
 * st_io.h -- typed pointer aliases for ST memory-mapped hardware.
 *
 * On the ST, the MIDI ACIA and YM2149 PSG registers sit at fixed
 * absolute addresses that Alcyon C would resolve via linker-bound
 * externs (e.g. `extern volatile char midictl;` with the symbol
 * assigned to 0xFFFC04 in the link map).  On the host we can't do
 * that, so we define each symbol as a macro expanding to a volatile
 * dereference: on the ST it hits the real hardware, on the host it
 * hits a static scratch byte (declared in psg_io.c) that has no
 * observable effect but keeps the code compilable.
 *
 * Register map:
 *
 *   MIDI ACIA (68901 MFP-adjacent):
 *     0xFFFC04  midictl   status/control (bit 1 = TDRE)
 *     0xFFFC06  midi      data
 *
 *   YM2149 PSG:
 *     0xFF8800  giselect  register select / read data
 *     0xFF8802  giwrite   write data (paired with giselect)
 */

#ifndef ST_IO_H
#define ST_IO_H

#ifdef HOST
/* Host: hardware doesn't exist; each register aliases a scratch byte
   defined in psg_io.c so the writes go somewhere real (avoids UB from
   volatile deref of NULL) but produce no output. */
extern volatile unsigned char   host_midictl_scratch;
extern volatile unsigned char   host_midi_scratch;
extern volatile unsigned char   host_giselect_scratch;
extern volatile unsigned char   host_giwrite_scratch;

#define midictl         host_midictl_scratch
#define midi            host_midi_scratch
#define giselect        host_giselect_scratch
#define giwrite         host_giwrite_scratch

#else
/* ST target: bind directly to the real hardware addresses. */
#define midictl         (*(volatile unsigned char *) 0xFFFC04L)
#define midi            (*(volatile unsigned char *) 0xFFFC06L)
#define giselect        (*(volatile unsigned char *) 0xFF8800L)
#define giwrite         (*(volatile unsigned char *) 0xFF8802L)
#endif

#endif  /* ST_IO_H */
