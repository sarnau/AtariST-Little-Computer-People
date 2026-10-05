#!/usr/bin/env python3
"""cp_encrypt.py -- encrypt checkCopyProt's track check in cp_asm.o.

cp_asm.s carries the 96 bytes between cpenc and cpencend as readable
instructions.  The original stores them encrypted: at run time cpdec1
subtracts a key from each word before the code is reached and cpenc1
adds it back afterwards.  alcyon_link.sh runs this script right after
assembling cp_asm.s, so the object -- and the linked LCP.PRG -- carry
the original's encrypted bytes.

The key is $1567, the value cpsetp's call into the cpsum chain leaves
in d0 (cpsum1 starts from a cleared d0 and the chain only adds
constants).  The block is 48 words because cpsetp loads $2f into d6
for its dbf loop.  Nothing in the block is relocated, so only the text
bytes change.

Safe to run twice: it encrypts only when the block still starts with
the plaintext first instruction, clr.l d7.

usage: cp_encrypt.py cp_asm.o
"""
import struct
import sys

KEY = 0x1567
WORDS = 0x2f + 1                # cpsetp: moveq #$2f,d6 / dbf d6
PLAIN_FIRST = 0x4287            # clr.l d7
HDR = 28                        # DRI object header


def symbols(obj):
    tsize, dsize, _bss, ssize = struct.unpack('>LLLL', obj[2:18])
    table = obj[HDR + tsize + dsize:HDR + tsize + dsize + ssize]
    syms = {}
    for i in range(0, len(table), 14):
        name = table[i:i + 8].rstrip(b'\0').decode('latin1')
        syms[name] = struct.unpack('>L', table[i + 10:i + 14])[0]
    return syms


def main(path):
    obj = bytearray(open(path, 'rb').read())
    if obj[:2] != b'\x60\x1a':
        sys.exit(f'{path}: not a DRI object')
    syms = symbols(obj)
    start, end = syms['cpenc'], syms['cpencend']
    if end - start != WORDS * 2:
        sys.exit(f'{path}: cpenc..cpencend is {end - start} bytes, '
                 f'the decrypt loop covers {WORDS * 2}')
    first = struct.unpack('>H', obj[HDR + start:HDR + start + 2])[0]
    if first != PLAIN_FIRST:
        print(f'{path}: track check already encrypted')
        return
    for off in range(HDR + start, HDR + end, 2):
        w = struct.unpack('>H', obj[off:off + 2])[0]
        obj[off:off + 2] = struct.pack('>H', (w + KEY) & 0xffff)
    open(path, 'wb').write(obj)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
