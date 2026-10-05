/*
 * midi_seq.c -- MIDI sequencer control surface.
 *
 * The 1985 game's MIDI subsystem lives in three tiers:
 *
 *   1. Song-lifecycle control (init/reset/start), header parsing
 *      dispatch, and playback-position bookkeeping.
 *
 *   2. The per-event MIDI parser and the PSG channel output driver
 *      (envelope stepping, note-on/off state, program change dispatch,
 *      tempo-derived tick divider), stepped from the Timer-A interrupt
 *      in mq_tick.s.
 *
 *   3. XBIOS/BIOS:  Midiws (send raw MIDI bytes) and Giaccess (PSG
 *      register write).  Both routed via _xbios in osbind.h.
 *
 * File-format provenance: .SNG and .ORG files are direct exports from
 * Activision Music Studio 2.0 (published 1986, Ed Bogas / Audio Light).
 * playSongFile strips a leading 10-byte Music Studio signature
 * (`\xCD` + "Mstudio" + `\xCD\x02`) before handing the rest of the file
 * to us; the layout below is offsets *inside the stripped body*, i.e.
 * inside the buffer playSongFile allocates.
 *
 * Stripped-body layout (relative to songEvents = start + 0x1FE):
 *
 *   body + 0x000..0x1A3    Music Studio config header:
 *                            +0x00..0x05  section tag "Blocks"
 *                            +0x1A..      instrument name list
 *                                         ("Harmonica", "Guitar", ...)
 *                            +0x??..      per-instrument ADSR envelope
 *                                         defaults, each 8 bytes
 *   body + 0x1A4..0x1FD    90-byte channel + program-change map
 *                          (15 logical channels x 2 bytes each; parsed
 *                          by unpackChanMap at p - 90)
 *   body + 0x1FE           MIDI event stream (this is songEvents)
 *
 * The order of the functions and parts/ includes in this file is the
 * original object's function order; keep it.
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "globals.h"
#include "protos.h"
#include "psgfreq.h"

/* skipTextField comes first in this object. */
#include "parts/skipTextField.c"

/* startSong: song-lifecycle entry point.
   If a song is playing: signal SEQ_PHASE_SONG_ENDING and return
   without starting the new one; caller spins until songPlaying is false.
   Idle: position songEvents at buffer+0x1FE (event stream), parse header,
   reset programs, skip 0x00/0xFF padding, store playback bounds, kick. */

void
startSong(song, maxPos)
unsigned char * song;
long            maxPos;
{

        if (songPlaying != NO) {
                seqPhase = SEQ_PHASE_SONG_ENDING;
                return;
        }

        parseSongHeader(songEvents = song + 0x1fe);
        resetPrograms();
        initSongState(skipTextField(songEvents), maxPos);
        armSequencer();
        songPlaying = YES;
}

/* initSongState: stash read cursor + end-of-song marker; init per-song
   driver state; publish ticks-per-beat via beatTicks.
   Envelope base = songEvents - 0x168 (360 bytes, ADSR block). */

void
initSongState(curPos, maxPos)
unsigned char * curPos;
long            maxPos;
{
        songPos = curPos;
        /* songEndPtr is the end-of-sequence pointer, and -1 is how "no
           limit" is spelled -- every caller passes songMaxPos, which
           is 0. */
        if (maxPos == 0)
                songEndPtr = (unsigned char *) -1L;
        else
                songEndPtr = (unsigned char *) maxPos;

        songAdsr = (long) (songEvents - 0x168);
        noteVel = defVelocity;
        noteVolume = defPsgVol;
        queueLen = 0;
        loopTop = 9;
        beatTicks = ticksPerBeat;
}

/* armSequencer: init timer counters + arm sequencer.
   All 4 tick counters seeded 100 (~500 ms grace before first event).
   seqBusy=0 selects XBIOS Midiws path (not direct ACIA). */

void
armSequencer()
{
        /* Chained assignments on purpose: separate statements
           compile differently. */
        seqBusy = timerTicks = 0;
        lastExpTick = nextEvTick = ticksToNext = seqCountdown = envDivider = 100;
        seqPhase = songActive = YES;
}

/* pushLoop: push loop marker {return position, count-1} on loopStack (cap 49). */

void
pushLoop(a, b)
void *  a;
short   b;
{
        if (loopTop < 49) {
                loopStack[loopTop] = (long) a;
                loopTop++;
                loopStack[loopTop] = (long)(short)(b - 1);
                loopTop++;
        }
}

/* popLoop: pop/decrement top of loop stack.  Returns loop-start ptr
   if count nonzero, else NULL (fall through end). */

unsigned char *
popLoop()
{
        unsigned char * ret;
        long            cnt;

        if (loopTop == 9)
                return (unsigned char *) 0;
        ret = (unsigned char *) loopStack[loopTop - 2];
        cnt = loopStack[loopTop - 1];
        loopStack[loopTop - 1]--;
        if (cnt == 0) {
                loopTop -= 2;
                return (unsigned char *) 0;
        } else
                return ret;
}

/* parseEvents: walk compact event stream at songPos.
   Byte forms:
     0x00        tick separator; returns 1, ticksToNext loaded
     0x01..0x7F  note event, 3 bytes:
                   byte0: [0..3]=logical ch, [4]=note-on (inverted),
                          [5]=sustain, [6]=note-off
                   byte1: [0..4]=dur index, [5]=accent (vel 0x7F),
                          [6..7]=transpose mode
                   byte2: MIDI note number
     0x82        bar marker (1 byte)
     0x85 <n>    loop start, count=n
     0x86        loop end (jump back if count > 0)
     0xFF        end of song, returns 0 */

short
parseEvents()
{
        /* No locals at all: songPos is walked with ++ in place and
           the command bytes are dispatched through a switch, as in the
           original. */

        /* Prologue: skip leading 0x00, refresh ticksToNext, end-check. */
        if (*songPos != 0)
                return 0;
        songPos++;
        if (songPos >= songEndPtr)
                return 0;
        noteDecoded = 0;
        peekNoteDur();
        if (songPos >= songEndPtr)
                return 0;

        while (*songPos != 0) {
                if ((*songPos & 0x80) == 0) {
                        /* Note event: unpack bytes 0..2, advance one
                           byte at a time, queue via queueNote.  byte1
                           bit 5 = accent (max vel + max PSG vol). */
                        noteDecoded = 1;
                        noteToQueue = 16 - (*songPos & 0x10);
                        mi_lasT = *songPos & 0x40;
                        noteIsOff = *songPos & 0x20;
                        noteChan = *songPos & 0x0f;
                        songPos++;

                        if ((noteAccent = *songPos & 0x20) != 0) {
                                noteVel = 0x7f;
                                noteVolume = 0xf;
                        } else {
                                noteVel = defVelocity;
                                noteVolume = defPsgVol;
                        }
                        noteMode = *songPos & 0xc0;
                        /* noteDur, not ticksToNext: this duration goes to
                           a second cell that only queueNote reads.
                           peekNoteDur computes the same expression into
                           ticksToNext, which the tick counters use. */
                        noteDur = (durTable[*songPos & 0x1f] - 1) * ticksPerBeat;
                        songPos++;

                        if ((noteMode & 0xc0) != 0) {
                                noteNum = *songPos & 0x7f;
                                songPos++;
                                if (noteMode & 0x80) {
                                        if (noteMode & 0x40)
                                                noteNum--;
                                        else
                                                noteNum++;
                                }
                        } else {
                                noteNum = noteMap[*songPos & 0x7f];
                                songPos++;
                        }

                        if (noteToQueue != 0)
                                queueNote();
                } else {
                        switch (*songPos++ & 0xff) {
                        case SEQ_BAR:
                                /* Bar marker: refresh ticksToNext for the
                                   next event only if none was decoded
                                   this pass. */
                                if (noteDecoded == 0) {
                                        peekNoteDur();
                                        if (songPos >= songEndPtr)
                                                return 0;
                                }
                                break;
                        case SEQ_LOOP_START:
                                /* Loop start: byte = count, push the
                                   return address. */
                                pushLoop(songPos + 1, *songPos);
                                songPos++;
                                peekNoteDur();
                                if (songPos >= songEndPtr)
                                        return 0;
                                break;
                        case SEQ_LOOP_END:
                                /* Loop end: pop, jump back if nonzero. */
                                if ((loopTarget = popLoop()) != 0)
                                        songPos = loopTarget;
                                peekNoteDur();
                                if (songPos >= songEndPtr)
                                        return 0;
                                break;
                        case SEQ_END:
                                return 0;
                                break;
                        }
                }
        }
        return 1;
}

/* peekNoteDur: skip 0x00 pad at songPos; peek next event's dur-index nibble.
   High-bit-clear (note) -> ticksToNext = tick-count; else ticksToNext = 0. */

void
peekNoteDur()
{
        for (; *songPos == 0; songPos++) ;
        if ((*songPos & 0x80) == 0)
                ticksToNext = (short)(durTable[(short)(char) songPos[1] & 0x1f]
                                                          - 1) * ticksPerBeat;
        else
                ticksToNext = 0;
}

/* queueNote: queue Note-On in noteQueue as {duration, note|sustain, physical channel}
   and dispatch Note-On via sendMidiEvent.  Queue entry fires paired Note-Off
   later via expireNotes + sendNoteOff. */

void
queueNote()
{
        short   ch;

        if (queueLen < 58) {
                noteQueue[queueLen] = noteDur;
                queueLen++;
                if (noteToQueue != 0) {
                        noteQueue[queueLen] = (mi_lasT << 1) | noteNum;
                        queueLen++;
                } else {
                        noteQueue[queueLen] = 0;
                        queueLen++;
                }
                noteQueue[queueLen] = chanMap[noteChan];
                queueLen++;
        } else
                return;

        if (noteNum > noteHigh)
                return;
        if (noteNum < noteLow)
                return;

        if (useSongChan != NO)
                sendProgChange(noteChan);
        else
                noteChan = fixedChan;

        if (noteIsOff != 0)
                noteOwner[noteNum] = 0;
        if (mi_lasT != 0)
                noteOwner[noteNum] = noteChan;
        if (noteIsOff != 0)
                return;

        ch = chanMap[noteChan];
        midiMsg[0] = (ch & 0xf) | 0x90;
        midiMsg[1] = noteNum;
        midiMsg[2] = noteVel;
        sendMidiEvent(midiMsg, (short) 3, ch);
}

/* sendNoteOff: send MIDI Note-Off (vel=0) for a queued note.
   nptr[0]={note|flags}, nptr[1]=physical channel byte.
   Fires only if note in [noteLow, noteHigh] and non-zero. */

void
sendNoteOff(nptr)
short * nptr;
{
        /* Called with &noteQueue[i] and walks the pointer forward.  The
           range test is a bitwise OR of two comparisons and each
           rejection returns a value from a void function; both are
           part of the original code. */
        if ((nptr[1] & 0x80) != 0)
                return;
        nptr++;
        midiMsg[1] = nptr[0];
        if ((char) midiMsg[1] > noteHigh | (char) midiMsg[1] < noteLow)
                return 1;
        if (midiMsg[1] == 0)
                return 1;
        nptr++;
        midiMsg[0] = (nptr[0] & 0xf) + 0x90;
        midiMsg[2] = 0;
        sendMidiEvent(midiMsg, (short) 3, (short) nptr[0]);
}

/* sendProgChange: dispatch Program Change (0xCn) for logical channel `index`.
   Fires only if cached program differs and MIDI output enabled.
   Current-program keyed by physical channel (chanMap & 0xf), so
   shared physical channels only get one PC per song load. */

void
sendProgChange(index)
char    index;
{
        if (sentProgram[chanMap[index] & 0xf] == progMap[index])
                return;
        if (midiOutOn == NO)
                return;

        midiMsg[0] = (chanMap[index] & 0xf) | 0xc0;
        midiMsg[1] = progMap[index];
        sentProgram[chanMap[index] & 0xf] = progMap[index];
        sendMidiEvent(midiMsg, (short) 2, (short) 0);
}

/* sendMidiEvent: send one MIDI event to MIDI OUT (Midiws) + YM2149 PSG.
   Both paths gated by their enabled flags.
   MIDI OUT: shift the note by (high nibble of midiCh - 3) octaves,
     write via aciaWrite (seqBusy=1) or Midiws; restore the note before
     the PSG path.
   PSG path (Note-On 0x9n only):
     vel=0 -> Note-Off: find channel by note, ENV_RELEASE.
     vel>0 -> Note-On: alloc silent channel, else voice-steal by
       highest phase; guard [noteHigh, noteLow]; copy 8 ADSR bytes from
       songAdsr + (noteChan-1)*8; take a (2 - N)*12 octave offset from
       attackDuration's high nibble N; write PSG tone/mixer/noise; if freq<0x17 use
       ENV_FADEOUT instead of ENV_ATTACK; set psgActive.
   Returns 1 on success, 0 on non-Note-On or Note-Off miss. */

short
sendMidiEvent(midiEvP, midiEvS, midiCh)
char *          midiEvP;
char            midiEvS;
char            midiCh;
{
        /* Both byte arguments are saved and restored around the MIDI
           OUT path, which walks them destructively.  The declaration
           order of these locals is part of the original code. */
        char            chosen;                 /* also the saved note */
        char            unused;                 /* unused, but it must stay */
        char            best;
        char            octShift;
        unsigned char * savedPtr;
        char            savedSize;
        long            envPtr;
        char            envelopePhase;
        short           attackHi;
        short           noiseMask;
        short           mixerBits;

        savedPtr  = midiEvP;
        savedSize = midiEvS;

        /* ---- MIDI OUT path ---- */
        if (midiOutOn != NO) {
                chosen = savedPtr[1];
                if (midiCh != 0)
                        midiEvP[1] = (midiEvP[1] & 0xff) -
                                (((3 - ((midiCh >> 4) & 0xf)) * 12) & 0xff);
                if (seqBusy == 1) {
                        while (midiEvS) {
                                aciaWrite(*midiEvP);
                                midiEvP++;
                                midiEvS--;
                        }
                } else {
                        Midiws(midiEvS - 1, midiEvP);
                }
                savedPtr[1] = chosen;
        }

        /* ---- PSG path ---- */
        if (psgOutOn != NO) {

                midiEvP = savedPtr;
                midiEvS = savedSize;

                if ((*midiEvP++ & 0xf0) != 0x90)
                        return 0;

                if (midiEvP[1] != 0) {

                /* ---- Note-On: pick a channel ---- */
                chosen = 0;
                while (psgChanNote[chosen++])
                        ;
                chosen--;
                if (chosen == 3) {
                        /* Voice-steal: pick the channel furthest along in its
                           envelope (highest phase index). */
                        best = chosen = 0;
                        while (chosen != 2) {
                                chosen++;
                                if (psgEnvelope[chosen].phase >
                                    psgEnvelope[chosen - 1].phase)
                                        best = chosen;
                        }
                        chosen = best;
                }

                /* Range guard: the whole note-on body is inside it.
                   The low limit is tested first, as in the original. */
                if (*midiEvP >= noteLow && *midiEvP <= noteHigh) {

                /* Copy 8 bytes of ADSR params from the .SNG envelope
                   block; the source address lands in a local first. */
                envPtr = (noteChan - 1) * 8 + songAdsr;
                envelopePhase = ENV_ATTACK;
                copyEnvelope(envPtr,
                        (unsigned char *) &psgEnvelope[chosen] + 1,
                        8L);

                /* Split the packed nibbles: attackStartVol keeps its low
                   4 bits (start volume), high 4 bits stash the mixer flags;
                   attackDuration keeps its low 4 bits, high 4 bits encode
                   the octave shift (2 - N) * 12 semitones. */
                attackHi = (psgEnvelope[chosen].attackStartVol >> 4) & 0xf;
                psgEnvelope[chosen].attackStartVol &= 0xf;
                octShift = (2 - ((psgEnvelope[chosen].attackDuration >> 4) & 0xf)) * 12;
                psgEnvelope[chosen].attackDuration &= 0xf;
                mixerBits = attackHi << chosen;
                noiseMask = ~(9 << chosen);

                /* The three scratch shorts are reused from here on:
                   attackHi carries the period, noiseMask its high
                   nibble and mixerBits the register number. */
                attackHi = psgPeriod[*midiEvP + octShift] / 60;
                if (seqBusy == 1) {
                        psgWrite(attackHi, PSG_NOISE_PERIOD);
                        psgMixer(mixerBits, noiseMask | 0xc0);
                } else {
                        /* The PSG writes go straight to the trap:
                           the Giaccess macro's (char) cast on the data
                           argument is not in the original here. */
                        xbios(XBIOS_GIACCESS, attackHi, PSG_WRITE | PSG_NOISE_PERIOD);
                        xbios(XBIOS_GIACCESS, xbios(XBIOS_GIACCESS, 0, PSG_MIXER) &
                                  (long) (noiseMask | 0xc0) |
                                  (long) mixerBits, PSG_WRITE | PSG_MIXER);
                }

                mixerBits = chosen << 1;

                if (*midiEvP + octShift > 22) {
                        attackHi = psgPeriod[*midiEvP + octShift];
                        noiseMask = (attackHi >> 8) & 0xf;
                        attackHi = attackHi & 0xff;
                        if (seqBusy == 1) {
                                psgWrite(attackHi, mixerBits);
                                psgWrite(noiseMask, mixerBits + 1);
                        } else {
                                xbios(XBIOS_GIACCESS, attackHi, mixerBits + PSG_WRITE);
                                xbios(XBIOS_GIACCESS, noiseMask, mixerBits + (PSG_WRITE | 1));
                        }
                } else {
                        envelopePhase = ENV_FADEOUT;
                }

                psgChanNote[chosen] = *midiEvP;
                if (envelopePhase == ENV_FADEOUT)
                        psgEnvelope[chosen].currentVolume = 0;
                psgEnvelope[chosen].maxVolume = noteVolume;
                psgActive = psgEnvelope[chosen].phaseTimer = 1;
                psgEnvelope[chosen].phase = envelopePhase;

                }       /* range guard */

                } else {

                /* ---- Note-Off (velocity == 0) ---- */
                chosen = 0;
                while (psgChanNote[chosen++] != *midiEvP && chosen < 4)
                        ;
                if (chosen == 4)
                        return 0;
                chosen--;
                psgChanNote[chosen] = 0;
                psgEnvelope[chosen].phase = ENV_RELEASE;
                psgEnvelope[chosen].phaseTimer = 0;

                }
                return 1;
        }

        return 1;
}

/* expireNotes: subtract val from each queued event's remaining duration;
   when <=0, sendNoteOff + removeQueued. */

void
expireNotes(val)
short   val;
{
        short   i;

        for (i = 0; i < queueLen; i += 3) {
                noteQueue[i] -= val;
                if (noteQueue[i] <= 0) {
                        sendNoteOff(&noteQueue[i]);
                        if (removeQueued(i) != 0)
                                i -= 3;
                }
        }
}

/* removeQueued must follow expireNotes directly. */
#include "parts/removeQueued.c"

/* timerAIsr lives in mq_tick.s: it needs privileged SR moves and an rte,
   which Alcyon C cannot emit. */

/* seqAdvance: sequencer state-machine advance from timerAIsr.
   WAIT_NOTE_EXPIRE (0): expire queued notes, reload prescaler, -> PARSE.
   PARSE_NEXT_EVENT (1): parseEvents() walks next batch; 0=end-of-song ->
     SONG_ENDING, else ticksToNext = ticks until next event.
   SONG_ENDING (2): expire remaining; when queue empty, kill PSG + flags. */

void
seqAdvance()
{
        short   res;

        if (seqPhase == SEQ_PHASE_WAIT_NOTE_EXPIRE) {
                res = timerTicks - lastExpTick;
                expireNotes(res);
                lastExpTick = timerTicks;
                seqCountdown = beatTicks;
                seqPhase = SEQ_PHASE_PARSE_NEXT_EVENT;
                nextEvTick += beatTicks;
                return;                 /* explicit return kept on purpose */
        } else if (seqPhase == SEQ_PHASE_PARSE_NEXT_EVENT) {
                seqPhase = SEQ_PHASE_WAIT_NOTE_EXPIRE;
                ticksToNext = -1;
                /* The parse sits in a loop that returns from both arms;
                   that shape is part of the original code. */
                while (ticksToNext < 0) {
                        if (parseEvents() != 0) {
                                nextEvTick += ticksToNext;
                                ticksToNext = nextEvTick - timerTicks;
                                if (ticksToNext > 0)
                                        seqCountdown = ticksToNext;
                                return;
                        } else {
                                seqPhase = SEQ_PHASE_SONG_ENDING;
                                seqCountdown = beatTicks;
                                nextEvTick += seqCountdown;
                                return;
                        }
                }
        } else {
                res = timerTicks - lastExpTick;
                expireNotes(res);
                lastExpTick = timerTicks;
                seqCountdown = beatTicks;
                nextEvTick += beatTicks;
                if (queueLen == 0) {
                        psgEnvelope[0].phase =
                        psgEnvelope[1].phase =
                        psgEnvelope[2].phase = ENV_IDLE;
                        songPlaying = psgActive = songActive = NO;
                        psgWrite(0, PSG_VOL_A);
                        psgWrite(0, PSG_VOL_B);
                        psgWrite(0, PSG_VOL_C);
                }
        }
}

/* stopSequencer: stop sequencer.
   Drain pending events, send Note-Off for every noteOwner[] flag,
   clear songActive.  Nothing calls it, but the original contains it. */

void
stopSequencer()
{
        short   note;
        short   hadPend;
        short   ch;

        if (queueLen > 0)
                hadPend = 1;
        else
                hadPend = 0;

        while (queueLen > 0) {
                ticksToNext = timerTicks - nextEvTick;
                if (ticksToNext > 0) {
                        expireNotes(ticksToNext);
                        nextEvTick += ticksToNext;
                }
        }

        if (hadPend != NO) {
                midiMsg[2] = 0;
                for (note = 0; note < 0x80; note++) {
                        if ((ch = noteOwner[note]) != 0) {
                                ch = chanMap[ch];
                                midiMsg[0] = (ch & 0x0f) | 0x90;
                                midiMsg[1] = note;
                                sendMidiEvent(midiMsg, 3, ch);
                        }
                }
        }

        songActive = NO;
}

/* hookTimerA sits between stopSequencer and unhookTimerA. */
#include "parts/hookTimerA.c"

/* unhookTimerA: tear down MFP Timer-A hook; Xbtimer(0,...) reinstalls
   saved ISR from oldTimerAVec.  Nothing calls it, but the original contains
   it. */

void
unhookTimerA()
{
        Xbtimer(XB_TIMER_A, MFP_STOP, 0x1c, oldTimerAVec);
}

/* resetPrograms and parseSongHeader come near the end of the object. */
#include "parts/resetPrograms.c"
#include "parts/parseSongHeader.c"

/* unpackChanMap: unpack 30-byte channel/program map (90 bytes before songEvents).
   Bytes 0..14 = MIDI channel for logical 1..15; bytes 15..29 = program.
   Values are 1-based on disk (0 = no-op sentinel); decrement on load.
   Logical channel 0 reserved for game SFX. */

void
unpackChanMap(p)
unsigned char * p;
{
        short   i;

        /* The offset arithmetic is written inside the dereference on
           purpose: `p[i - 1]` compiles differently. */
        for (i = 1; i < 16; i++) {
                chanMap[i] = *(p + i - 1)  - 1;
                progMap[i] = *(p + i + 14) - 1;
        }
}

/* Rebuild noteMap, the note translation table timerAIsr reads every note
   through, from the key setting in a song's header (parseSongHeader passes the
   byte it also stores in songKey).  Starts from identity, marks five
   entries of the lowest octave 0xFF, and returns there for value 1.
   Otherwise, in every octave, each scale degree whose bit is CLEAR in
   keyScaleMask[value] (bit 0 = B ... bit 6 = C) is moved one semitone: up
   for values up to 8, down above 8 -- i.e. sharps or flats. */
void
buildNoteMap(value)
short   value;
{
        short           i;
        short           noteShift;
        char            chordMask;

        for (i = 0; i < 0x84; i++)
                noteMap[i] = i;
        noteMap[1]  = -1;
        noteMap[3]  = -1;
        noteMap[6]  = -1;
        noteMap[8]  = -1;
        noteMap[10] = -1;

        if (value == 1)
                return 1;

        if (value > 8)
                noteShift = -1;
        else
                noteShift = 1;

        for (i = 0; i < 0x84; i += 12) {
                chordMask = keyScaleMask[value];
                if ((chordMask & 1) == 0)
                        noteMap[i + 11] += noteShift;
                chordMask >>= 1;
                if ((chordMask & 1) == 0)
                        noteMap[i + 9] += noteShift;
                chordMask >>= 1;
                if ((chordMask & 1) == 0)
                        noteMap[i + 7] += noteShift;
                chordMask >>= 1;
                if ((chordMask & 1) == 0)
                        noteMap[i + 5] += noteShift;
                chordMask >>= 1;
                if ((chordMask & 1) == 0)
                        noteMap[i + 4] += noteShift;
                chordMask >>= 1;
                if ((chordMask & 1) == 0)
                        noteMap[i + 2] += noteShift;
                chordMask >>= 1;
                if ((chordMask & 1) == 0)
                        noteMap[i] += noteShift;
        }
}

/* copyEnvelope must sit right before stepEnvelopes. */
#include "parts/copyEnvelope.c"

/* stepEnvelopes: PSG software ADSR envelope processor, run by timerAIsr
   every fourth Timer-A tick (240 Hz).
   3 channels through attack->decay->sustain->release->fadeout.
   Per phase: Bresenham accum, delta = (target-cur)*envRateTab[t],
   accum += delta; while accum > 360, cur += dir; accum -= 360.
   phaseTimer==0 with dur==0 -> immediate fall-through (gotos).
   Clamp cur to maxVolume; write PSG amp reg 8/9/10 via psgWrite.
   The case fall-throughs are written as gotos, as in the original. */

void
stepEnvelopes()
{
        char    i;

        for (i = 0; i < 3; i++) {
                if (!psgEnvelope[i].phase)
                        continue;

                switch (psgEnvelope[i].phase) {
                case ENV_ATTACK:
                        psgEnvelope[(short) i].currentVolume =
                                                 psgEnvelope[(short) i].attackStartVol;
                        psgEnvelope[(short) i].phase = ENV_DECAY;
                        if (!psgEnvelope[i].attackDuration) {
                                psgEnvelope[(short) i].currentVolume =
                                                                 psgEnvelope[(short) i].attackTargetVol;
                                psgEnvelope[(short) i].phaseTimer = 0;
                                goto do_decay;
                        }
                        psgEnvelope[(short) i].phaseTimer =
                                                 (short) psgEnvelope[(short) i].attackDuration;
                        if (psgEnvelope[i].attackStartVol >
                            psgEnvelope[i].attackTargetVol) {
                                rampDelta[(short) i] =
                                                          (short) psgEnvelope[(short) i].attackStartVol -
                                                          (short) psgEnvelope[(short) i].attackTargetVol;
                                psgEnvelope[(short) i].rampDirection = -1;
                        } else {
                                psgEnvelope[(short) i].rampDirection = 1;
                                rampDelta[(short) i] =
                                                          (short) psgEnvelope[(short) i].attackTargetVol -
                                                          (short) psgEnvelope[(short) i].attackStartVol;
                        }
                        rampDelta[(short) i] = rampDelta[(short) i] *
                                             envRateTab[psgEnvelope[(short) i].phaseTimer];
                        psgEnvelope[(short) i].phaseTimer =
                                             envTimeTab[psgEnvelope[(short) i].phaseTimer];
                        rampAccum[(short) i] = 0;
                        break;

                case ENV_DECAY:
do_decay:
                        if (psgEnvelope[i].phaseTimer-- > 0) {
                                rampAccum[i] += rampDelta[i];
                                while (rampAccum[i] > 0x168) {
                                        psgEnvelope[i].currentVolume +=
                                                psgEnvelope[i].rampDirection;
                                        rampAccum[i] -= 0x168;
                                }
                                break;
                        } else {
                                if (!psgEnvelope[i].decayDuration) {
                                        psgEnvelope[(short) i].currentVolume =
                                                                          psgEnvelope[(short) i].decayTargetVol;
                                        psgEnvelope[(short) i].phaseTimer = 0;
                                        goto do_sustain;
                                }
                                psgEnvelope[(short) i].phase = ENV_SUSTAIN;
                                psgEnvelope[(short) i].phaseTimer =
                                                                 (short) psgEnvelope[(short) i].decayDuration;
                                if (psgEnvelope[i].attackTargetVol >
                                    psgEnvelope[i].decayTargetVol) {
                                        rampDelta[(short) i] =
                                                                  (short) psgEnvelope[(short) i].attackTargetVol -
                                                                  (short) psgEnvelope[(short) i].decayTargetVol;
                                        psgEnvelope[(short) i].rampDirection = -1;
                                } else {
                                        psgEnvelope[(short) i].rampDirection = 1;
                                        rampDelta[(short) i] =
                                                                  (short) psgEnvelope[(short) i].decayTargetVol -
                                                                  (short) psgEnvelope[(short) i].attackTargetVol;
                                }
                                rampDelta[(short) i] = rampDelta[(short) i] *
                                                                          envRateTab[psgEnvelope[(short) i].phaseTimer];
                                psgEnvelope[(short) i].phaseTimer =
                                                                          envTimeTab[psgEnvelope[(short) i].phaseTimer];
                                rampAccum[(short) i] = 0;
                                break;
                        }

                case ENV_SUSTAIN:
do_sustain:
                        if (psgEnvelope[i].phaseTimer-- > 0) {
                                rampAccum[i] += rampDelta[i];
                                while (rampAccum[i] > 0x168) {
                                        psgEnvelope[i].currentVolume +=
                                                psgEnvelope[i].rampDirection;
                                        rampAccum[i] -= 0x168;
                                }
                                break;
                        } else {
                                if (!psgEnvelope[i].sustainDuration) {
                                        psgEnvelope[(short) i].currentVolume =
                                                                          psgEnvelope[(short) i].sustainTargetVol;
                                        psgEnvelope[(short) i].phaseTimer = 0;
                                        goto do_release;
                                }
                                psgEnvelope[(short) i].phase = ENV_RELEASE;
                                psgEnvelope[(short) i].phaseTimer =
                                                                 envSusTab[(short) psgEnvelope[(short) i].sustainDuration];
                                if (psgEnvelope[i].decayTargetVol >
                                    psgEnvelope[i].sustainTargetVol) {
                                        rampDelta[(short) i] =
                                                                  (short) psgEnvelope[(short) i].decayTargetVol -
                                                                  (short) psgEnvelope[(short) i].sustainTargetVol;
                                        psgEnvelope[(short) i].rampDirection = -1;
                                } else {
                                        psgEnvelope[(short) i].rampDirection = 1;
                                        rampDelta[(short) i] =
                                                                  (short) psgEnvelope[(short) i].sustainTargetVol -
                                                                  (short) psgEnvelope[(short) i].decayTargetVol;
                                }
                                rampDelta[(short) i] = rampDelta[(short) i] *
                                                                          envRelTab[(short) psgEnvelope[(short) i].sustainDuration];
                                rampAccum[(short) i] = 0;
                                break;
                        }

                case ENV_RELEASE:
do_release:
                        if (psgEnvelope[i].phaseTimer-- > 0) {
                                rampAccum[i] += rampDelta[i];
                                while (rampAccum[i] > 0x168) {
                                        psgEnvelope[i].currentVolume +=
                                                psgEnvelope[i].rampDirection;
                                        rampAccum[i] -= 0x168;
                                }
                                break;
                        } else {
                                if (psgEnvelope[i].releaseDuration) {
                                        psgEnvelope[(short) i].phase = ENV_FADEOUT;
                                        psgEnvelope[(short) i].phaseTimer =
                                                         (short) psgEnvelope[(short) i].releaseDuration;
                                        rampDelta[(short) i] =
                                                  (short) psgEnvelope[(short) i].currentVolume;
                                        psgEnvelope[(short) i].rampDirection = -1;
                                        rampDelta[(short) i] = rampDelta[(short) i] *
                                                  envRateTab[psgEnvelope[(short) i].phaseTimer];
                                        psgEnvelope[(short) i].phaseTimer =
                                                  envTimeTab[psgEnvelope[(short) i].phaseTimer];
                                        rampAccum[(short) i] = 0;
                                        break;
                                } else {
                                        psgEnvelope[(short) i].phaseTimer = 0;
                                        goto do_fadeout;
                                }
                        }
                        /* falls through into ENV_FADEOUT */

                case ENV_FADEOUT:
do_fadeout:
                        if (psgEnvelope[i].phaseTimer-- > 0 &&
                            psgEnvelope[i].currentVolume) {
                                rampAccum[i] += rampDelta[i];
                                while (rampAccum[i] > 0x168) {
                                        psgEnvelope[i].currentVolume +=
                                                psgEnvelope[i].rampDirection;
                                        rampAccum[i] -= 0x168;
                                }
                        } else {
                                psgEnvelope[i].currentVolume =
                                        psgEnvelope[i].phase = ENV_IDLE;
                        }
                        break;
                }

                /* The clamped volume goes through a global, not a
                   local, and the pick is a ternary (one store); both
                   are part of the original code. */
                envOutVol = psgEnvelope[i].currentVolume >
                           psgEnvelope[i].maxVolume
                         ? psgEnvelope[i].maxVolume
                         : psgEnvelope[i].currentVolume;
                psgWrite(envOutVol, ampRegs[i] - PSG_WRITE);
        }
}
