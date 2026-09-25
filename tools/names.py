"""Build build/names.tsv, the names the recompiler gives Conduit 2's stripped DOL.

Four sources, most trusted first:

  tools/names-manual.tsv   names given by hand, each with its evidence
  natives                  the strat engine's {"ass_Name", fn} tables
                           (tools/natives.py): strat_Name
  elf                      build/elf_names.tsv from tools/elfmatch.py: the
                           same SDK builds as Victorious's symbolised ELF
  signature                build/sig_guess.tsv from sigmatch.py (Dolphin's
                           database), unique names only

An address keeps the first name it gets; a name is given once.

    python tools/elfmatch.py build/extract/sys/main.dol <Victorious ELF> > build/elf_names.tsv
    python tools/names.py build/extract/sys/main.dol
"""
import collections
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))  # wiikit/

from wiikit.dol import Image
from tools.natives import natives

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")


def read_tsv(path, name_col):
    out = []
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            next(f)
            for line in f:
                p = line.rstrip("\n").split("\t")
                if len(p) > name_col and p[0].strip() and not p[0].startswith("#"):
                    out.append((int(p[0], 16), p[name_col]))
    return out


def unique(pairs):
    by_name, by_addr = collections.defaultdict(set), collections.defaultdict(set)
    for a, n in pairs:
        by_name[n].add(a)
        by_addr[a].add(n)
    return {a: next(iter(ns)) for a, ns in by_addr.items()
            if len(ns) == 1 and len(by_name[next(iter(ns))]) == 1}


def main():
    img = Image(sys.argv[1])
    manual = dict(read_tsv(os.path.join(ROOT, "tools", "names-manual.tsv"), 1))
    native = {a: "strat_" + n for a, n in natives(img).items()}
    elf = dict(read_tsv(os.path.join(ROOT, "build", "elf_names.tsv"), 2))
    sig = unique((a, n.split("(")[0].replace(" ", "_").replace("::", "__"))
                 for a, n in read_tsv(os.path.join(ROOT, "build", "sig_guess.tsv"), 2))
    names, source, taken = {}, {}, set()
    for src, table in (("manual", manual), ("native", native), ("elf", elf), ("signature", sig)):
        for a, n in table.items():
            if a in names or n in taken:
                continue
            names[a], source[a] = n, src
            taken.add(n)
    out = os.path.join(ROOT, "build", "names.tsv")
    with open(out, "w", encoding="utf-8") as f:
        f.write("addr\tname\tsource\n")
        for a in sorted(names):
            f.write(f"{a:08X}\t{names[a]}\t{source[a]}\n")
    print(f"{len(names)} names -> {out}: {dict(collections.Counter(source.values()))}")


if __name__ == "__main__":
    main()
