"""Name a stripped DOL's functions from a symbolised ELF built from the same libraries.

Conduit 2 links the very same RVL SDK builds as Victorious (every
"<< RVL_SDK - X release build: ... >>" string equal, Aug 23 2010), and
Victorious shipped an ELF with 20 619 sized function symbols. So instead of
guessing function starts and hashing them (tools/sigmatch.py), every sized
function of the ELF is looked for at every word of the DOL's text:

  * words are compared with what the linker changes masked: the 24-bit
    target of `b`/`bl`, the 16-bit immediate of addi/addis/ori/oris and of
    every D-form load and store (lis/addi pairs, SDA offsets). Conditional
    branches are relative inside the function and kept;
  * a function found at exactly one address, whose address matches no other
    name, is named. A name found at several addresses (identical helpers) or
    an address with several names (identical functions) is dropped;
  * then, to a fixed point, the functions too small or too common to be
    unique (KPADRead is `li; b KPADiRead`): a candidate is kept if every
    `b`/`bl` in it whose target is named on both sides goes to the same name,
    at least one does, and the candidate is then the only one for its name.

    python tools/elfmatch.py build/extract/sys/main.dol ELF [--min 16] > build/elf_names.tsv
"""
import argparse
import collections
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))  # wiikit/

from wiikit.dol import Image, library_of

IMM_OPS = {14, 15, 24, 25} | set(range(32, 56))
KEY = 4                                            # words hashed for the first lookup


def mask(w):
    op = w >> 26
    if op == 18:
        return w & 0xFC000003
    if op in IMM_OPS:
        return w & 0xFFFF0000
    return w


def masked_words(data):
    return [mask(w) for w in struct.unpack(">%dI" % (len(data) // 4), data)]


def branches(img, addr, size):
    """[(offset, target)] of the b/bl instructions in a function."""
    out = []
    for k in range(0, size, 4):
        w = img.u32(addr + k)
        if w >> 26 == 18 and not w & 2:
            li = w & 0x03FFFFFC
            if li & 0x02000000:
                li -= 0x04000000
            out.append((k, (addr + k + li) & 0xFFFFFFFF))
    return out


def anchored(dol, elf, named, size_of, lib_of):
    """Small and repeated functions, told apart by where they branch."""
    elf_name_at = {s.addr: s.name for s in elf.functions()}
    small = [s for s in elf.functions() if 4 <= s.size <= 64 and not s.size % 4]
    while True:
        dol_addr = {n: a for a, n in named.items()}
        taken = set(dol_addr)
        added = 0
        for s in small:
            if s.name in taken:
                continue
            br = branches(elf, s.addr, s.size)
            anchors = [(k, elf_name_at.get(t)) for k, t in br]
            anchors = [(k, dol_addr[n]) for k, n in anchors if n in dol_addr]
            if not anchors:
                continue
            sw = masked_words(elf.read(s.addr, s.size))
            k0, t0 = anchors[0]
            hits = candidates_branching_to(dol, t0, k0, sw)
            hits = [a for a in hits if all(branch_at(dol, a + k) == t for k, t in anchors)]
            hits = [a for a in hits if a not in named]
            if len(hits) == 1:
                named[hits[0]] = s.name
                size_of[s.name], lib_of[s.name] = s.size, library_of(s)
                taken.add(s.name)
                added += 1
        if not added:
            return


_callers = None


def candidates_branching_to(dol, target, k, sw):
    """DOL addresses a, with a `b`/`bl` at a+k to target, whose masked words equal sw."""
    global _callers
    if _callers is None:
        _callers = collections.defaultdict(list)
        for seg in dol.text_segments():
            for a, t in branches(dol, seg.vaddr, len(seg.data)):
                _callers[t].append(a)
    out = []
    for c in _callers.get(target, ()):
        a = c - k
        try:
            if masked_words(dol.read(a, 4 * len(sw))) == sw:
                out.append(a)
        except Exception:
            pass
    return out


def branch_at(img, a):
    w = img.u32(a)
    if w >> 26 != 18:
        return None
    li = w & 0x03FFFFFC
    if li & 0x02000000:
        li -= 0x04000000
    return (a + li) & 0xFFFFFFFF


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dol")
    ap.add_argument("elf")
    ap.add_argument("--min", type=int, default=16, help="smallest function, in bytes")
    a = ap.parse_args()
    dol, elf = Image(a.dol), Image(a.elf)

    index = collections.defaultdict(list)          # first KEY masked words -> [(sym, words)]
    for s in elf.functions():
        if s.size < max(a.min, 4 * KEY) or s.size % 4:
            continue
        try:
            w = masked_words(elf.read(s.addr, s.size))
        except Exception:
            continue
        index[tuple(w[:KEY])].append((s, w))
    print("# %d ELF functions indexed" % sum(len(v) for v in index.values()), file=sys.stderr)

    found = collections.defaultdict(set)           # name -> addrs
    at = collections.defaultdict(set)              # addr -> names
    size_of, lib_of = {}, {}
    for seg in dol.text_segments():
        w = masked_words(seg.data)
        for i in range(len(w) - KEY + 1):
            cands = index.get(tuple(w[i:i + KEY]))
            if not cands:
                continue
            for s, sw in cands:
                if w[i:i + len(sw)] == sw:
                    addr = seg.vaddr + 4 * i
                    found[s.name].add(addr)
                    at[addr].add(s.name)
                    size_of[s.name] = s.size
                    lib_of[s.name] = library_of(s)
    named = {}
    for addr, names in at.items():
        if len(names) == 1:
            n = next(iter(names))
            if len(found[n]) == 1:
                named[addr] = n
    first = len(named)
    anchored(dol, elf, named, size_of, lib_of)
    print("# %d unique by their words, %d more by their branch targets" % (first, len(named) - first),
          file=sys.stderr)
    libs = collections.Counter(lib_of[n] for n in named.values())
    print("# %d names found, %d unique both ways" % (len(found), len(named)), file=sys.stderr)
    print("# by library: " + ", ".join("%s %d" % kv for kv in libs.most_common(40)), file=sys.stderr)
    print("addr\tsize\tname\tlibrary")
    for addr in sorted(named):
        n = named[addr]
        print("%08X\t0x%X\t%s\t%s" % (addr, size_of[n], n, lib_of[n]))


if __name__ == "__main__":
    main()
