/*
 * assets.c -- OBJECTS/SPRITES/BODY.LCP/PEx.LCP/NAMES loaders and
 * the dispatchers that unpack them into runtime MFDB tables.
 *
 * OBJECTS/SPRITES record: {h:BE16, w:BE16, ceil(w/16)*4*2*h pixel bytes}
 *   (4 bitplanes interleaved per row, MSB-first).  File caps at 14000.
 * BODY.LCP / PE2..PE6.LCP: {count:BE16, total_bytes:BE16, payload}
 *   168 bytes per 16x21 frame (21 rows x 4 words = 2 image + 2 mask).
 * NAMES: newline-terminated ASCII, <= 10 chars per line.
 *
 * addr: ldObj(), ldSpr()
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
#include <osbind.h>
#include "alerts.h"
#include "assets.h"
#include "globals.h"
#include "save.h"
#include "sprender.h"
#include "sprglobs.h"
#include "sprites.h"


/* ldObj -> parts/ldObj.c (STX: 0x524a). */
#ifdef FAITHFUL
#include "parts/ldObj.c"
#endif

/* ldSpr -> parts/ldSpr.c (STX: 0x528a). */
#ifdef FAITHFUL
#include "parts/ldSpr.c"
#endif

/* Parse OBJECTS/SPRITES buffer -> per-record MFDB + w/h arrays.
   Stops at buffer end / height==0 / 64 records. */

static short
prsRec(buf, size, mfdb_tab, w_tab, h_tab)
unsigned char * buf;
long            size;
MFDB *          mfdb_tab;
short *         w_tab;
short *         h_tab;
{
        long    offset;
        short   count;
        short   width;
        short   height;
        long    words_per_row;
        long    record_bytes;

        offset = 0;
        count  = 0;
        while (offset < size && count < 64) {
                /* BE16 read, host-endian agnostic. */
                height = ((short) buf[offset]     << 8) | buf[offset + 1];
                width  = ((short) buf[offset + 2] << 8) | buf[offset + 3];
                if (height == 0 || width == 0)
                        break;
                offset = offset + 4;

                /* MFDB width MUST be a multiple of 16 (sp_iniM does
                   fd_wdwidth = width/16 with truncation).  Round up. */
                words_per_row = (width + 15) / 16;
                sp_iniM(0L, &mfdb_tab[count],
                                 buf + offset,
                                 (short) (words_per_row * 16), height);
                /* w_tab keeps unrounded pixel width for od_draw's src rect. */
                w_tab[count] = width;
                h_tab[count] = height;

                /* Advance: words_per_row * 4 planes * 2 bytes. */
                record_bytes  = (long) height * words_per_row * 4 * 2;
                offset = offset + record_bytes;
                count = count + 1;
        }
        return count;
}

/* al_loot: read OBJECTS and unpack.  Port-side wrapper; ROM inlines
   at 0x15546 as ldObj() + 56-iter parse loop. */

short
al_loot()
{
        ldObj();
        return prsRec(obj_file, 14000L,
                             g_obtmt,
                             g_obtaw, g_obtah);
}

/* al_lost: read SPRITES and unpack.  Port-side wrapper. */

short
al_lost()
{
        ldSpr();
        return prsRec(spr_file, 14000L,
                             g_setmt,
                             g_setaw, g_setah);
}

/* al_loal: load BODY.LCP / PE2..6.LCP into caller buffer.
   Header: {count:BE16, total_bytes:BE16, payload}.  Returns frame count. */

short
al_loal(filename, dest_buf, max_b)
char *          filename;
unsigned char * dest_buf;
long            max_b;
{
        short           fhnd;
        unsigned char   header[4];
        short           count;
        long            total;

        fhnd = fOpen(filename, 0);
        fr_read(fhnd, 4L, header);
        count = ((short) header[0] << 8) | header[1];
        total = ((long)  header[2] << 8) | header[3];
        if (total > max_b)
                total = max_b;
        fr_read(fhnd, total, dest_buf);
        Fclose(fhnd);
        return count;
}

/* al_locs: load BODY.LCP + PEx.LCP (x = character_sprite_id, 2..6,
   clamped to 2).  Wires body_ptr and pex_ptr.  Static buffers
   (survive to game end without heap fragmentation). */

/* body.lcp @ 0x3f8b0 = 20160 B, pex_lcp_file @ 0x4d2da = 11088 B
   (168 bytes/frame, sp_lcpf w=2/h=21). */
/* LCP_STX reads both files straight into the global frame arrays
   (body_ptr / pex_ptr), so there are no staging buffers. */

void
al_locs()
{
        /* Ghidra pex_name @ 0x2a0f8 mutates index 2 in place; port uses
           stack-local (GEMDOS FAT case-insensitive). */
        char    pex_filename[8];        /* "PEn.LCP\0" */
        short   which;

        /* Round buffer caps, not exact file sizes. */
        al_loal("body.lcp", (unsigned char *) body_ptr, 20000L);

        which = lcp.character_sprite_id;
        if (which < 2 || which > 6)
                which = 2;

        pex_filename[0] = 'P';
        pex_filename[1] = 'E';
        pex_filename[2] = '0' + which;
        pex_filename[3] = '.';
        pex_filename[4] = 'L';
        pex_filename[5] = 'C';
        pex_filename[6] = 'P';
        pex_filename[7] = 0;

        al_loal(pex_filename, (unsigned char *) pex_ptr, 12000L);
}

/* unScn: decode .SCN screen image into out_wds (16-bit words).
   Nibble-stream like fr_reac, but 15-WORD dictionary at file offset
   2..31 (30 bytes), 0xF escape reads 4 more nibbles for literal word.
   Header 32 bytes; payload at 32.
   addr: decompress_scn @ ROM 0x15546 (with wrapper fOpen/Malloc/etc). */

void
unScn(filename, out_wds, dst_wds)
char *          filename;
unsigned short *out_wds;
long            dst_wds;
{
        short           filehandle;
        unsigned char * fbuffer;
        long            body_size;
        long            i;
        long            count;
        unsigned short  word_dict[15];
        unsigned char   sizebuf[2];

        filehandle = fOpen(filename, 0);
        fr_read(filehandle, 2L, sizebuf);
        body_size = ((long) sizebuf[0] << 8) | sizebuf[1];

        {
                unsigned char raw[30];
                unsigned short hi;
                unsigned short lo;
                fr_read(filehandle, 30L, raw);
                for (i = 0; i < 15; i = i + 1) {
                        /* Byte-mask before OR: Alcyon 4.14 sign-extends
                           raw[j] with bit 7 set (0xFF -> 0xFFFF). */
                        hi = (unsigned short) raw[i * 2]     & 0x00FF;
                        lo = (unsigned short) raw[i * 2 + 1] & 0x00FF;
                        word_dict[i] = (hi << 8) | lo;
                }
        }

        body_size = body_size - 32;
        fbuffer = (unsigned char *) Malloc(body_size);
        if (fbuffer == (unsigned char *) 0)
                er_nomem();
        fr_read(filehandle, body_size, fbuffer);

        /* Nibble state-machine decode.  `b = *src & 0xFF` isolates the
           byte before Alcyon signed-char promotion in shifts. */
        {
                short           readHigh;
                unsigned short  val;
                unsigned short  nibble;
                short           j;
                unsigned char * src;
                unsigned short  b;

                src = fbuffer;
                readHigh = 1;
                for (count = 0; count < dst_wds; count = count + 1) {
                        b = (unsigned short) (*src) & 0x00FF;
                        if (readHigh) {
                                val = (b >> 4) & 0x0F;
                        } else {
                                val = b & 0x0F;
                                src = src + 1;
                        }
                        readHigh = 1 - readHigh;
                        nibble = val;

                        if (nibble == 0x0F) {
                                nibble = 0;
                                for (j = 0; j < 4; j = j + 1) {
                                        b = (unsigned short) (*src) & 0x00FF;
                                        if (readHigh) {
                                                val = (b >> 4) & 0x0F;
                                        } else {
                                                val = b & 0x0F;
                                                src = src + 1;
                                        }
                                        readHigh = 1 - readHigh;
                                        nibble = (nibble << 4) | val;
                                }
                                out_wds[count] = nibble;
                        } else {
                                out_wds[count] = word_dict[nibble];
                        }
                }
        }
        Fclose(filehandle);
        Mfree(fbuffer);
}

/* al_loan: read NAMES text file (plain ASCII, newline-terminated names,
   ~2.6 KB on the 1985 disk). */

long
al_loan(dest_buf, max_b)
unsigned char * dest_buf;
long            max_b;
{
        short   fhnd;

        fhnd = fOpen("names", 0);
        fr_read(fhnd, max_b, dest_buf);
        Fclose(fhnd);
        return max_b;
}
