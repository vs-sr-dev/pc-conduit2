# The executable

`sys/main.dol`, 6.3 MB, entry `0x80004050`, BSS `0x80603020`+`0x1CB380`.
No REL modules (the seven RELs on the disc belong to Nintendo's MotionPlus
video, `sys/mpls_movie/player.dol`, a separate program).

| Section | Range | Size |
|---|---|---|
| T0 | 80004000–80006720 | 10 KB (`__start`, init, runtime asm) |
| T1 | 8000AFE0–80574C60 | 5.42 MB |
| D0–D7 | 80006720…8060C9A0 | ~0.6 MB data, 1.8 MB BSS |

**Stripped**, like Dragon Quest Swords, but three times its size and close to
Victorious's: **1 421 544 words** of code (Victorious 1.66 M, DQS 0.74 M).

## What is linked

From the version strings (`<< RVL_SDK - X release build: … >>`):

| Library | Build | Notes |
|---|---|---|
| RVL SDK: AI, AX, DSP, DVD, EXI, GX, KPAD, NAND, OS, PAD, SC, SI, VI, WPAD, ENC | **Aug 23 2010** (0x4302_145) | byte for byte the builds Victorious links: every string equal |
| HBM | Jul 30 2010 | as Victorious |
| VEN | Jul 15 2010 | |
| DWC | Aug 27 2010 | Nintendo Wi-Fi Connection |
| NHTTP, SSL, NCD, NWC24, SO, SOCKET | May–Jun 2009 | the network stack below DWC; WiiConnect24 |
| Scaleform GFx | | the menus and HUD (ActionScript), a different version from Victorious's |
| Bink | | the seven movies, a different version from Victorious's |
| Quazal (Net-Z, Rendez-Vous) | | the online game: `Quazal::…`, `JobConnectStation`, `DOCallContext.cpp`, `PRUDPEndPoint.cpp` |
| GameSpy | | `sdkdev.gamespy.com`, `gamespygp` (through DWC) |
| Wii Speak | | `ass_WiiSpeak*`, `nb_celp.c` (the speech codec) |
| MSL C, Runtime, TRK | CodeWarrior | |

Above them is **High Voltage Software's engine**: C++, cross-platform (Xbox,
PC and PSP code paths are still named in it: `XbBindController`,
`PcSetConfig`, `PcRawMouseMove` stubbed on the Wii), with its own audio
layer (`AAL::`), its own "Diesel" and "Quantum" heaps, `GCNFile`,
`GCNSkin.cpp`, `GCNcControllerManager.cpp` (GameCube-era names), and a
**script language, "strats"** (compiled `.SVM`, one per WAD), that drives
the whole game from the level logic to the menus.

## Size and shape of the code

`tools/census.py` of Dragon Quest Swords, run on this DOL:

* 1 421 544 words, 176 distinct operations, **14 non-zero words**
  undecoded (all data inside text, around `__start`): wiikit's decoder
  covers this compiler's output as it did Victorious's and DQS's.
* **Paired singles 0.55%**: 7 764 `psq_*`, 6 044 on GQR0 (plain float
  pairs), **134 quantised** (GQRs 2–6).
* 10 689 distinct `bl` targets (83 199 calls), 11 938 `stwu r1`
  prologues, **9 491 `bcctr`**: a lot of virtual C++ and function pointers
  (DQS 2 581).

## Naming without symbols

Three sources, merged by `tools/names.py` into `build/names.tsv`
(**4 846 names**):

| Source | Names | How |
|---|---|---|
| **The strat engine's own tables** (`tools/natives.py`) | **2 575** | the script language calls the engine through tables of `{char* "ass_Name", void* fn}`: every native, from `ControllerAnalogRX` to `WiiMotionPlusCalibrateZeroPoint`, named from the executable itself, one function each |
| **Victorious's ELF** (`tools/elfmatch.py`) | **2 217** | the SDK builds are the same, so every sized function of Victorious's symbolised ELF is looked for at every word of this text, relocations masked, unique both ways: RVL SDK 645, free functions 624 (MSL, math, SDK helpers), Bluetooth 545, Home Button 258, MSL/runtime 72, GFx 30, Bink 10. 20 s |
| Dolphin's signature database (`sigmatch.py` of DQS) | 53 more | a crude first pass; everything it found that matters, the ELF found too |

`elfmatch` is the better tool whenever a symbolised executable shares a
library build: `GXInit` (321 words), `VIInit` (338), `OSCreateThread`
(155) match word for word. It misses what the linker left out: **`KPADRead`
is not linked here**; the game reads through `KPADReadEx` (801C7820,
`li r7,1; b KPADiRead`), named by hand.

Of the runtime's 47 hook names, 26 resolve through signatures, and the
rest are functions this executable does not link (`KPADInit`, `KPADReset`,
low-level WPAD). Two it does link and the runtime did not know,
`KPADInitEx` and `KPADReadEx`, are now in wiikit (`10-wiikit.md`).

The game's own code (8000B000–~80160000) stays nameless except the
natives; the next names come from RTTI strings, `GCNcControllerManager`
and the natives' callees, given by hand as the port needs them.

## Recompiled (session 1)

    python -m wiikit.recomp build/extract/sys/main.dol --out build/recomp \
        --symbols build/names.tsv --hooks tools/conduit2-hooks.txt

**19 421 units, no gaps**, 19 455 entries, 9 917 data pointers into code;
175 switch tables sized from the code, 2 unresolved (both in
`__ptmf_scall`, the pointer-to-member trampoline: its `bctr` goes through
the dispatch table anyway); 2 bad targets; 13 s. **173 files of C++ that
compile and link with no error** (clang -O1, about 70 s): the recompiler
took a stripped executable of Victorious's size without a change.

## Landmarks

* **The main loop**, 8005E87C–8005F2D4: the `MPLS_REBOOT` checks (the
  return from the MotionPlus video), DWC's fatal-error screen (a message
  table in five languages, then a loop forever), a frame limiter that
  yields until 1/fps has passed when a flag is set (8005F284), the frame
  (8003A6B0) ending in `GXDrawDone`.
* **The controller manager**, 8010DE40–8010E600: per channel it sets up KPAD
  (`KPADSetConnectCallback` with the callback **800158F4**,
  `KPADSetPosParam`...), reads with `KPADReadEx` (8010E000), probes with
  `WPADProbe` (8010E2B0), rumbles with `WPADControlMotor`. State for 8
  controllers of 0x28 bytes each at `[r13-0x3414] + 0x3044`: what the
  natives `ControllerAnalogLX/LY/RX/RY`, `ControllerAnalogLTrigger`… return.
* **The connect callback** 800158F4 registers an empty extension callback
  (800158F0, a lone `blr`), treats channel 3 as the Balance Board, and
  initialises the channel's state.
* A network thread (Net-Z / DWC, 8012D7B4…) polls with `OSYieldThread`.
