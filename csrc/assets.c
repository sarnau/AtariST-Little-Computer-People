/*
 * assets.c -- asset file loaders (OBJECTS, SPRITES, BODY.LCP, PEx.LCP,
 *             NAMES, and the dispatchers that unpack them into runtime
 *             MFDB tables).
 *
 * Formats (all big-endian, all extracted from readFiles.py + Ghidra
 * load_objects / load_sprites decompiles):
 *
 *   OBJECTS / SPRITES:
 *     Sequence of records, each:
 *       +0..1     height (short, big-endian)
 *       +2..3     width  (short, big-endian)
 *       +4..     ceil(width/16) * 4 * 2 * height  pixel bytes
 *                (4 bitplanes interleaved per row, MSB-first)
 *     Total file size caps at 14000 bytes; parser stops at buffer end.
 *
 *   BODY.LCP / PE2..PE6.LCP (character sprite sheets):
 *     +0..1     count  (short, big-endian) -- number of frames
 *     +2..3     total  (short, big-endian) -- total payload bytes
 *                        (== count * 168 for the 16x21 LCP sprites)
 *     +4..      total pixel bytes -- 168 bytes per 16x21 frame,
 *               laid out as 21 rows of 4 words (2 image + 2 mask).
 *
 *   Historical note: the previous version of this file documented the
 *   second short as "bytes per frame" and al_loal multiplied
 *   count * size to get the read length -- that gave the correct
 *   number for OBJECTS/SPRITES-style records but a nonsense-huge
 *   value for BODY.LCP (98 * 16464 = ~1.6 MB) which happened to be
 *   capped by max_bytes at the call site.  Real semantics: the
 *   header's second short IS the total payload byte count.  See
 *   tests/sprite_compose.c for the byte-verified layout.
 *
 *   NAMES:
 *     Plain ASCII, newline-terminated names, one per line.  10 chars
 *     max per name.  Read as text.
 *
 * addr: load_objects(), load_sprites()
 *       (parsers + name/BODY/PEx loaders inferred from readFiles.py)
 */

#include "types.h"
#include "structs.h"
#include "enums.h"
/* --- per-file extern block (auto-generated for Alcyon).
       For the monolithic "everything" view see
       include/globals.h.  Alcyon C 4.14 has a fixed-size
       symbol table that overflows on the full globals.h. */
extern PLAYER   lcp;                            /* the resident LCP */
extern unsigned char    objects_file[];
extern unsigned char    sprites_files[];
extern MFDB     g_obtmt[];
extern MFDB     g_setmt[];
extern short    g_obtaw[];
extern short    g_obtah[];
extern short    g_setaw[];
extern short    g_setah[];
extern short *  pex_lcp_file;                   /* source head sheet */
extern short *  body_lcp_file;
extern short *  body_shape_data;
extern short *  head_shape_data;
extern short    body_shape_data_buf[];
extern short    head_shape_data_buf[];
#include <osbind.h>

extern short    file_open();
extern void     fr_read();
extern void     sp_iniM();
extern void     error_not_enough_memory();

/* load_objects: read the 14000-byte OBJECTS file into objects_file[].
   addr: load_objects() */

void
load_objects()
{
        short   fileHandle;

        fileHandle = file_open("objects", 0);
        fr_read(fileHandle, 14000L, objects_file);
        _gemdos(GEMDOS_Fclose, (long) fileHandle, 0L, 0L);
}

/* load_sprites: read the 14000-byte SPRITES file into sprites_files[].
   addr: load_sprites() */

void
load_sprites()
{
        short   fileHandle;

        fileHandle = file_open("sprites", 0);
        fr_read(fileHandle, 14000L, sprites_files);
        _gemdos(GEMDOS_Fclose, (long) fileHandle, 0L, 0L);
}

/* Parse a sequence-of-records buffer (OBJECTS or SPRITES format) into
   its per-record MFDB descriptor table + width/height arrays.  Stops
   at buffer end (records with height==0 or offset >= size) or at the
   64th record (table size). */

static short
parse_records(buf, size, mfdb_tab, w_tab, h_tab)
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
                /* Height + width are big-endian shorts.  Read via a
                   two-byte splice to keep the port host-endian
                   agnostic. */
                height = ((short) buf[offset]     << 8) | buf[offset + 1];
                width  = ((short) buf[offset + 2] << 8) | buf[offset + 3];
                if (height == 0 || width == 0)
                        break;
                offset = offset + 4;

                sp_iniM(0L, &mfdb_tab[count],
                                 buf + offset, width, height);
                w_tab[count] = width;
                h_tab[count] = height;

                /* Advance past this record's pixel data: ceil(w/16)
                   words per row * 4 planes * 2 bytes per word. */
                words_per_row = (width + 15) / 16;
                record_bytes  = (long) height * words_per_row * 4 * 2;
                offset = offset + record_bytes;
                count = count + 1;
        }
        return count;
}

/* al_loot: read OBJECTS, then unpack the records. */

short
al_loot()
{
        load_objects();
        return parse_records(objects_file, 14000L,
                             g_obtmt,
                             g_obtaw, g_obtah);
}

/* al_lost: read SPRITES, then unpack the records. */

short
al_lost()
{
        load_sprites();
        return parse_records(sprites_files, 14000L,
                             g_setmt,
                             g_setaw, g_setah);
}

/* al_loal: load a BODY.LCP / PE2..PE6.LCP character sprite
   sheet into a caller-supplied buffer.  Header format:
     +0..1  count (short, big-endian) -- number of frames
     +2..3  total (short, big-endian) -- total payload bytes
                    (== count * 168 for the 16x21 LCP sprites)
   Returns the number of frames.
   addr: (inferred; the 1985 code has one loader per file) */

short
al_loal(filename, dest_buf, max_bytes)
char *          filename;
unsigned char * dest_buf;
long            max_bytes;
{
        short           fileHandle;
        unsigned char   header[4];
        short           count;
        long            total;

        fileHandle = file_open(filename, 0);
        fr_read(fileHandle, 4L, header);
        count = ((short) header[0] << 8) | header[1];
        total = ((long)  header[2] << 8) | header[3];
        if (total > max_bytes)
                total = max_bytes;
        fr_read(fileHandle, total, dest_buf);
        _gemdos(GEMDOS_Fclose, (long) fileHandle, 0L, 0L);
        return count;
}

/* al_locs: boot-time entry that loads BODY.LCP and
   the PEx.LCP head sheet keyed by the PLAYER's character_sprite_id
   into their runtime buffers, then wires body_lcp_file and pex_lcp_file
   at those buffers.

   Buffers are static (not GEMDOS_Malloc'd) so the load survives to
   game end without heap fragmentation.  BODY.LCP holds 98 x 168-byte
   frames = 16464 bytes; each PEx sheet holds up to 66 x 168-byte
   head frames = 11088 bytes.  Sized to 20000 / 12000 to leave headroom
   for any unseen variant.

   Called from the boot sequence after al_lost and
   before the first sp_updb / sp_lchu tick.

   The character_sprite_id is 2..6, matching PE2..PE6.LCP.  Values
   outside that range are clamped to 2 so the loader never wanders off
   a random string ("PE1.LCP" doesn't exist in the shipped disk).

   addr: (inferred; the 1985 loader is inlined in the boot path with a
   direct filename-string switch on character_sprite_id) */

static unsigned char    body_lcp_buffer[20000];
static unsigned char    pex_lcp_buffer[12000];

void
al_locs()
{
        char    pex_filename[8];        /* "PEn.LCP\0" */
        short   which;

        al_loal("body.lcp", body_lcp_buffer,
                       (long) sizeof(body_lcp_buffer));
        body_lcp_file    = (short *) body_lcp_buffer;
        body_shape_data  = body_shape_data_buf;

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

        al_loal(pex_filename, pex_lcp_buffer,
                       (long) sizeof(pex_lcp_buffer));
        pex_lcp_file    = (short *) pex_lcp_buffer;
        head_shape_data = head_shape_data_buf;
}

/* decompress_scn: decode a .SCN screen-image file into
   `out_words` (16-bit output words, dest_size measured in words, not
   bytes).  Same nibble-stream shape as fr_reac, but two
   width differences:
     - Dictionary is 15 *words* (30 bytes) at file offset 2..31,
       vs 15 bytes at offset 2..16 for the .TXT variant.
     - Each output symbol is a 16-bit word: recognised nibbles map
       through the word dictionary, and the escape (nibble == 0xF)
       reads 4 more nibbles for a literal 16-bit word.
   Total header = 32 bytes; payload starts at offset 32.

   HOUSE.SCN and TITLE.SCN are the two 320x200 4-plane screen images
   that the 1985 game boots from (house background and title splash).

   addr: (inferred from Python decompressImageFile; the 1985 loader
   is likely inlined in the intro/setup path) */

void
decompress_scn(filename, out_words, dest_size_words)
char *          filename;
unsigned short *out_words;
long            dest_size_words;
{
        short           filehandle;
        unsigned char * fbuffer;
        unsigned char * fbuffer_orig;
        long            body_size;
        long            i;
        long            count;
        unsigned short  word_dict[15];
        unsigned short  flag;
        unsigned short  nibble;
        unsigned short  literal;
        unsigned char   sizebuf[2];

#ifdef __ALCYON__
        gemdos(9, "  scn.a\r\n");     /* Cconws marker A: entered */
#endif
        filehandle = file_open(filename, 0);
#ifdef __ALCYON__
        gemdos(9, "  scn.b\r\n");     /* B: file opened */
#endif
        fr_read(filehandle, 2L, sizebuf);
#ifdef __ALCYON__
        gemdos(9, "  scn.c\r\n");     /* C: size read */
#endif

        /* File size is big-endian.  Reassemble explicitly for host
           portability; on the ST this is a no-op. */
        body_size = (long) (((unsigned long) sizebuf[0] << 8) |
                            sizebuf[1]);

        /* Read the 30-byte (15-word) dictionary. */
        {
                unsigned char raw[30];
                fr_read(filehandle, 30L, raw);
                for (i = 0; i < 15; i = i + 1)
                        word_dict[i] = (unsigned short)
                                (((unsigned long) raw[i * 2] << 8) |
                                 raw[i * 2 + 1]);
        }

        /* Read the compressed body.  Total header = 32 bytes, so body
           length is fileSize - 32.  Allocate + slurp. */
#ifdef __ALCYON__
        gemdos(9, "  scn.d dict done\r\n");
#endif
        body_size = body_size - 32;
        fbuffer = (unsigned char *) _gemdos(GEMDOS_Malloc,
                                            body_size, 0L, 0L);
        fbuffer_orig = fbuffer;
#ifdef __ALCYON__
        gemdos(9, "  scn.e malloc\r\n");
#endif
        if (fbuffer == (unsigned char *) 0)
                error_not_enough_memory();
        fr_read(filehandle, body_size, fbuffer);
#ifdef __ALCYON__
        gemdos(9, "  scn.f body read\r\n");
#endif

        /* DIAGNOSTIC: try one write to out_words first.  If this crashes,
           the pointer or buffer is bad.  If it doesn't, the decode loop
           is the crash. */
#ifdef __ALCYON__
        gemdos(9, "  scn.g try write out_words[0]\r\n");
#endif
        out_words[0] = 0x1234;
#ifdef __ALCYON__
        gemdos(9, "  scn.h wrote\r\n");
#endif
#ifdef __ALCYON__
        gemdos(9, "  scn.i try write out_words[15999]\r\n");
#endif
        out_words[15999] = 0x5678;
#ifdef __ALCYON__
        gemdos(9, "  scn.j wrote last word\r\n");
#endif

        /* Decode.  Same nibble state-machine as fr_reac,
           just wider symbols. */
        /* Simple sequential nibble reader: unpack all body bytes into
           a nibble buffer up front, then walk the nibble stream.
           Avoids the flag state-machine that c168 mis-compiles. */
        {
                unsigned char * np;
                unsigned char * nbuf;
                long           bn;
                long           ni;
                unsigned short lit;

#ifdef __ALCYON__
                gemdos(9, "  scn.k unpack nibbles\r\n");
#endif
                nbuf = (unsigned char *) _gemdos(GEMDOS_Malloc,
                                                 body_size * 2L, 0L, 0L);
                if (nbuf == (unsigned char *) 0)
                        error_not_enough_memory();
                np = nbuf;
                for (bn = 0; bn < body_size; bn = bn + 1) {
                        *np++ = (fbuffer_orig[bn] >> 4) & 0x0f;
                        *np++ = fbuffer_orig[bn] & 0x0f;
                }

#ifdef __ALCYON__
                gemdos(9, "  scn.l1 decode start\r\n");
#endif
                np = nbuf;
#ifdef __ALCYON__
                gemdos(9, "  scn.l2 first read np\r\n");
#endif
                {
                        unsigned char first = *np;
                        char m[8];
                        m[0] = ' '; m[1] = 'n'; m[2] = '=';
                        m[3] = '0' + (char)(first / 10);
                        m[4] = '0' + (char)(first % 10);
                        m[5] = '\r'; m[6] = '\n'; m[7] = 0;
#ifdef __ALCYON__
                        gemdos(9, m);
#endif
                }
#ifdef __ALCYON__
                gemdos(9, "  scn.l3 read word_dict\r\n");
#endif
                {
                        unsigned short wd = word_dict[1];
#ifdef __ALCYON__
                        gemdos(9, "  scn.l4 word_dict[1] ok\r\n");
#endif
                        out_words[0] = wd;
#ifdef __ALCYON__
                        gemdos(9, "  scn.l5 wrote out_words[0]\r\n");
#endif
                }
                for (count = 0; count < dest_size_words; count = count + 1) {
                        if (*np == 0x0f) {
                                np = np + 1;
                                lit = *np++;
                                lit = (lit << 4) | *np++;
                                lit = (lit << 4) | *np++;
                                lit = (lit << 4) | *np++;
                                out_words[count] = lit;
                        } else {
                                out_words[count] = word_dict[*np];
                                np = np + 1;
                        }
                }
#ifdef __ALCYON__
                gemdos(9, "  scn.m decode done\r\n");
#endif
                _gemdos(GEMDOS_Mfree, (long) nbuf, 0L, 0L);
        }

        _gemdos(GEMDOS_Fclose, (long) filehandle, 0L, 0L);
        _gemdos(GEMDOS_Mfree,  (long) fbuffer_orig, 0L, 0L);
}

/* al_loan: read the NAMES text file into a caller-provided
   buffer.  Format is plain ASCII, one name per line, newline
   terminated.  The buffer is a raw ASCII dump; the caller (name
   selection logic) walks it line-by-line for random pick.
   NAMES file on the 1985 disk is 2.6 KB; we read up to that much. */

long
al_loan(dest_buf, max_bytes)
unsigned char * dest_buf;
long            max_bytes;
{
        short   fileHandle;

        fileHandle = file_open("names", 0);
        fr_read(fileHandle, max_bytes, dest_buf);
        _gemdos(GEMDOS_Fclose, (long) fileHandle, 0L, 0L);
        return max_bytes;
}
