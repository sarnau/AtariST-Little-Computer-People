"""Minimal Atari GEMDOS PRG reader: segments + relocation-masked image."""
import struct

class PRG:
    def __init__(self, path):
        d = open(path, 'rb').read()
        self.raw = d
        (magic, self.tsz, self.dsz, self.bsz,
         self.ssz, _res, self.flags, self.absflag) = struct.unpack('>HIIIIIIH', d[:28])
        assert magic == 0x601a, path
        self.text = d[28:28+self.tsz]
        self.data = d[28+self.tsz:28+self.tsz+self.dsz]
        self.image = bytearray(d[28:28+self.tsz+self.dsz])   # text+data as loaded (TPA-relative)
        self.reloc = []
        off = 28 + self.tsz + self.dsz + self.ssz
        if not self.absflag and off + 4 <= len(d):
            first = struct.unpack('>I', d[off:off+4])[0]
            off += 4
            if first:
                pos = first
                self.reloc.append(pos)
                while off < len(d):
                    b = d[off]; off += 1
                    if b == 0:
                        break
                    if b == 1:
                        pos += 254
                    else:
                        pos += b
                        self.reloc.append(pos)
        self.masked = bytearray(self.image)
        for p in self.reloc:
            if p + 4 <= len(self.masked):
                self.masked[p:p+4] = b'\x00\x00\x00\x00'

    def where(self, off):
        return ('text', off) if off < self.tsz else ('data', off - self.tsz)
