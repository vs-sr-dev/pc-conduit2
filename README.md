# pc-conduit2

Toward a native PC port of **Conduit 2** (Wii, High Voltage Software /
Sega, 2011), the first-person shooter. It was a Wii exclusive and never
re-released. The goal is the game running natively on PC and played with
mouse and keyboard: the Classic Controller's left stick on WASD, its right
stick on the mouse, and gamepads as Classic Controllers.

The route is static recompilation: the game's own PowerPC code, translated
to C++ and built for the PC, running on a replacement for the Wii's
hardware and system software. Nothing is emulated at the instruction
level, and nothing of the game is rewritten.

This repository documents the disc, its formats and its code, and holds the
port's own tools and layer. It is the third port built on
**[wiikit](https://github.com/vs-sr-dev/wiikit)**, the game-agnostic Wii
toolkit that grew with [pc-victorious](https://github.com/vs-sr-dev/pc-victorious)
and [pc-dragonquestswords](https://github.com/vs-sr-dev/pc-dragonquestswords),
taken here as a submodule at `wiikit/` (clone with `--recursive`, or
`git submodule update --init`). Where this game needs something every Wii
game would need (a NAND race in IOS, the GP FIFO in MEM2, the Classic
Controller, SDL gamepads), it goes into wiikit, not here.

## Where it stands

After three sessions the game **boots, draws, sounds and plays to its
first level**: the logos and Bink movies, the title, the menus, a profile,
the prologue and the oil rig, at 16:9, characters whole and colours right.
It is played with **keyboard and mouse, a gamepad, or both at once**, and
the game's **split-screen** works with the keys and mouse as player 1 and
a pad as player 2. **It is not yet played through**: nothing past the first
level has been tried. Fog is not drawn, new sights stutter the first time
they appear (shaders compiled on first use), and mouse look still goes
through the right stick, so the turn rate has the stick's cap. The buttons
cannot yet be remapped on the pad. See [Status](#status) and
[docs/07-next-session.md](docs/07-next-session.md).

## BYOA — Bring Your Own Assets

This repository contains **documentation and tools only**. No game data, no
executables, no assets. You need your own original disc. The work is done on
the North American release, SC2E8P (English, French, Spanish); the
addresses in `tools/` and `docs/` are that executable's.

## Layout

    docs/     disc, format and code analysis, the plan, the session log
    tools/    Conduit 2-specific tools, the port's layer (conduit2.cpp)
    wiikit/   game-agnostic Wii toolkit (submodule: github.com/vs-sr-dev/wiikit)
    build/    (not in git) the disc, everything derived from it, the build

## Tools

The Python tools need only Python 3.8+ and no dependencies (pycryptodome,
if installed, speeds up disc decryption). Building the recompiled code
needs CMake, Ninja, a C++20 compiler (clang from MSYS2 is what is used
here) and SDL3; running it needs OpenGL 4.5. Run from the repository root.

```sh
# the disc (.wbfs, .iso or .rvz)
python -m wiikit.disc GAME.wbfs --info
python -m wiikit.disc GAME.wbfs --extract build/extract

# the executable: stripped, so its names are found, not read
python -m wiikit.dol build/extract/sys/main.dol --info
python tools/natives.py build/extract/sys/main.dol   # the strat engine's natives
python tools/names.py build/extract/sys/main.dol     # -> build/names.tsv
```

`names.py` alone names what the build needs: the strat engine's own tables
name 2 575 functions, and `tools/names-manual.tsv` the rest the runtime
hooks, each with its evidence. For reading the code, the SDK can be named
too: the game links the very same SDK builds as Victorious, so
`tools/elfmatch.py` finds 2 217 of the functions of Victorious's
symbolised ELF (`Oscar_wii_final_versioned.elf`, on that game's disc) word
for word, into `build/elf_names.tsv`; signatures from Dolphin's database
(`sigmatch.py` of pc-dragonquestswords, into `build/sig_guess.tsv`) are
taken as well. Run `names.py` again after either.

```sh
python tools/elfmatch.py build/extract/sys/main.dol <Victorious ELF> > build/elf_names.tsv

# recompile, build: the port's hooks, and its layer through WIIKIT_EXTRA
python -m wiikit.recomp build/extract/sys/main.dol --out build/recomp \
    --symbols build/names.tsv --hooks tools/conduit2-hooks.txt
cmake -S build/recomp -B build/recomp-build -G Ninja -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE=-O1 \
    -DWIIKIT_EXTRA=$PWD/tools/conduit2.cmake
ninja -C build/recomp-build

# boot the game: the NAND (saves, SYSCONF) in build/nand; the boot ROM's
# fonts and the DSP ROM's resampling table in build/fonts (font_western.bin,
# font_japanese.bin, dsp_coef.bin: Dolphin's Sys/GC has free ones)
build/recomp-build/wiiboot build/extract --symbols build/names.tsv
build/recomp-build/wiiboot build/extract --window 1920x1080
build/recomp-build/wiiboot build/extract --input keyboard   # split-screen: pads from player 2

# where the time goes, and a frame's GX commands and EFB (see wiikit)
WIIKIT_PERF=1 build/recomp-build/wiiboot build/extract 2> run.err
CONDUIT2_TRACE=1 build/recomp-build/wiiboot build/extract    # the WAD streamer, the NAND, skinning
```

`tools/dolphin.py` runs the disc in Dolphin as the reference, with
screenshots and key presses on a timeline (`--dolphin PATH`, or `DOLPHIN`
in the environment).

### Playing

The game plays with a Classic Controller, which the port makes out of the
keyboard, the mouse and any gamepad. Channel 1 merges the three: buttons
from any of them, each stick from whichever is pushed further. Every other
pad plugged in is the next channel. `--input pad` or `--input keyboard`
keeps one source on channel 1 (with `keyboard`, the pads start at channel
2: the game's split-screen).

| Keys, mouse | Gamepad | Classic Controller |
|---|---|---|
| W A S D | left stick | left stick: move |
| mouse movement | right stick | right stick: look |
| left button | right trigger | ZR: fire |
| right button | left trigger | ZL: aim |
| Enter, Space | right face button | A |
| Backspace, C | bottom face button | B |
| R | top face button | X |
| F | left face button | Y |
| Left Shift, E | shoulders | L, R |
| Tab, Q | Start, Back | +, - |
| arrows | d-pad | d-pad |
| H | guide | Home |
| Esc, F11 / Alt+Enter, F12 | | pause box (frees the mouse), fullscreen, trace a frame's GX commands |

The pad's face buttons count by position, as on a Classic Controller (the
right one is A); `Face Buttons = Label` in the key file counts them by
their labels instead. The keys are in `build/keys.txt`, written with
wiikit's defaults on the first run; its `[Classic Controller]` section
changes them. `CONDUIT2_MOUSE` sets the mouse's gain (0.15).

## Status

Session 3: **gamepads, and the Classic Controller in wiikit.** The port's
own Classic emulation moved into wiikit, game-agnostic: the Classic's
status on every channel, the connect and extension callbacks called when a
controller comes or goes, SDL3 gamepads as Classic Controllers, the key
file's Classic section, channel 1 merging keys, mouse and the first pad,
the Remote's motor rumbling the pad. The port's layer keeps only the mouse
on the right stick. Played with an Xbox One pad, with keys and mouse
alongside, and in split-screen.

Session 2: **the game plays.** The black screen after the legal notices
was a race in wiikit's IOS (a NAND lookup for downloaded patches); past it,
the logos, the title, the menus, a profile, the prologue and the first
level, played with WASD and the mouse. Characters, skinned on the CPU into
MEM2, were half missing until three GP FIFO bugs in wiikit were fixed; the
game's colour grading was green-for-blue until RG8/GB8 EFB copies were.

Session 1: **analysis, plan, first light.** The disc is mapped (265 WADs,
Bink movies). The executable is stripped but its SDK is Victorious's own
build: 4 846 functions named (2 575 by the script engine's own tables,
2 217 word for word from Victorious's ELF). It recompiles to C++ that
compiles and links at the first try (19 421 functions), and boots: the
Wii Strap, "A Nunchuk or Classic Controller is required", the legal
notices; then a black screen. See
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
