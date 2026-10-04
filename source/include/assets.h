/* assets.h -- the object/sprite and frame-file loaders.  Their bodies
   live in parts/ (ldObj, ldSpr, al_loal); main unpacks most assets
   itself.

   File formats:
   OBJECTS, SPRITES: records of {h: BE16, w: BE16, then ceil(w/16)*4*2*h
     pixel bytes} -- 4 bitplanes interleaved per row, MSB first.  The
     loaders stop at the buffer end, at height 0 or after 64 records.
   BODY.LCP, PE2..PE6.LCP: {count: BE16, total_bytes: BE16, payload};
     168 bytes per 16x21 frame (21 rows x 4 words: 2 image + 2 mask).
     BODY.LCP is 20160 bytes, a PEx.LCP 11088.  PEx is chosen by
     character_sprite_id (2..6).
   NAMES: fixed 10-byte records; lcp_crnd seeks to a random one.
   .SCN: a nibble stream with a 15-word dictionary in bytes 2..31 of a
     32-byte header; nibble 0xF escapes to four more nibbles forming a
     literal word.  The payload starts at byte 32. */

#ifndef ASSETS_H
#define ASSETS_H

extern void ldObj();
extern void ldSpr();
extern short al_loal();

#endif /* ASSETS_H */
