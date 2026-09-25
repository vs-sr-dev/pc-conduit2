"""Name the strat engine's native functions from its own tables.

Conduit 2's script language ("strats", compiled .SVM) calls into the engine
through tables of {char* name, void* fn} pairs, the name being the native's
with an "ass_" prefix. Every pair whose name is such a string and whose
function lies in text is a named function. One function behind many names is
a shared stub ("only exists in P_TABLE as stub!") and is left out.

    python tools/natives.py build/extract/sys/main.dol > build/natives.tsv
"""
import collections
import os
import re
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))  # wiikit/

from wiikit.dol import Image


def natives(img):
    """{fn: name} (without "ass_") for every native with a function of its own."""
    text = [(s.vaddr, s.vaddr + len(s.data)) for s in img.text_segments()]
    in_text = lambda a: any(lo <= a < hi for lo, hi in text)
    strs = {}
    for s in img.segments:
        if not s.text:
            for m in re.finditer(rb"ass_[A-Za-z0-9_]+\x00", s.data):
                strs[s.vaddr + m.start()] = m.group()[:-1].decode()
    pairs = collections.defaultdict(set)          # fn -> names
    tables = collections.Counter()
    for s in img.segments:
        if s.text:
            continue
        for k in range(0, len(s.data) - 7, 4):
            n, f = struct.unpack_from(">II", s.data, k)
            if n in strs and in_text(f):
                pairs[f].add(strs[n][4:])
                tables[(s.vaddr + k) >> 16] += 1
    named = {f: next(iter(ns)) for f, ns in pairs.items() if len(ns) == 1}
    if not __name__ == "__main__":
        return named
    print("# %d names in %d functions; %d functions shared by several names (stubs) left out"
          % (sum(len(v) for v in pairs.values()), len(pairs), len(pairs) - len(named)), file=sys.stderr)
    return named


def main():
    named = natives(Image(sys.argv[1]))
    for f in sorted(named):
        print("%08X\tstrat_%s" % (f, named[f]))


if __name__ == "__main__":
    main()
