# pc-conduit2

Toward a native PC port of **Conduit 2** (Wii, High Voltage Software /
Sega, 2011), the first-person shooter. It was a Wii exclusive and never
re-released. The goal is the game running natively on PC and played with
mouse and keyboard: the Classic Controller's left stick on WASD, its right
stick on the mouse.

This repository documents the disc, its formats and its code, and grows the
tooling for the port. It is the third port built on
**[wiikit](https://github.com/vs-sr-dev/wiikit)**, the game-agnostic Wii
toolkit of [pc-victorious](https://github.com/vs-sr-dev/pc-victorious),
taken here as a submodule at `wiikit/` (clone with `--recursive`, or
`git submodule update --init`). Where this game needs something every Wii
game would need, it goes into wiikit, not here.

## BYOA — Bring Your Own Assets

This repository contains **documentation and tools only**. No game data, no
executables, no assets. You need your own original disc. The work is done on
the North American release, SC2E8P (English, French, Spanish).

## Layout

    docs/     disc, format and code analysis, the plan
    tools/    Conduit 2-specific tools and the port's layer
    wiikit/   game-agnostic Wii toolkit (submodule: github.com/vs-sr-dev/wiikit)

## Tools

Python 3.8+, no dependencies (pycryptodome, if installed, speeds up disc
decryption). Run from the repository root. The build as in
`docs/07-next-session.md`: clang, Ninja and SDL3 from MSYS2.

```sh
python -m wiikit.disc GAME.wbfs --extract build/extract
python tools/natives.py build/extract/sys/main.dol          # the strat natives
python tools/elfmatch.py build/extract/sys/main.dol <Victorious ELF> > build/elf_names.tsv
python tools/names.py build/extract/sys/main.dol            # -> build/names.tsv
python -m wiikit.recomp build/extract/sys/main.dol --out build/recomp \
    --symbols build/names.tsv --hooks tools/conduit2-hooks.txt
```

## Status

Session 1: **analysis, plan, first light.** The disc is mapped (265 WADs,
Bink movies). The executable is stripped but its SDK is Victorious's own
build: 4 846 functions named (2 575 by the script engine's own tables,
2 217 word for word from Victorious's ELF). It recompiles to C++ that
compiles and links at the first try (19 421 functions), and boots: the
Wii Strap, "A Nunchuk or Classic Controller is required", the legal
notices; then a black screen, next. See
[docs/06-attack-plan.md](docs/06-attack-plan.md).

## Documentation

    00-sessions.md            progress log
    01-disc-layout.md         what is on the disc
    03-executable.md          the stripped DOL: what is linked, how it is named, landmarks
    04-curiosities.md         what the disc reveals
    05-open-questions.md      what is still unknown
    06-attack-plan.md         feasibility and the porting route
    07-next-session.md        the plan for the next session
    08-input.md               controllers in this game, and the mouse-and-keyboard scheme
    10-wiikit.md              how this port uses and grows wiikit

## Licence

MIT. This covers the documentation and tools in this repository only. It
says nothing about Conduit 2, which remains the property of its rights
holders.
