# Feasibility and the porting route

## Verdict

**Feasible, by the route of Victorious and Dragon Quest Swords**: static
recompilation of the whole executable to C++, with the Wii SDK replaced at
the hardware by wiikit's runtime. Session 1 went further than a plan: the
stripped DOL is named, recompiled, compiled, linked and **booted to its
first screens** (the Wii Strap, "A Nunchuk or Classic Controller is
required", the legal notices), with sound running on the AX.

| Easier than feared | Why |
|---|---|
| **The SDK is Victorious's** | every RVL SDK library is the same Aug 2010 build: the runtime's hardware, AX, VI, GX, DVD, IOS paths are the ones Victorious already exercised for seven sessions |
| **Names for free** | 2 575 engine functions named by the script engine's own tables; 2 217 library functions by Victorious's ELF, word for word |
| **The middleware is known** | Scaleform GFx and Bink, as in Victorious (other versions, but recompiled like any code; the runtime drew both) |
| **The controls are a pad's** | the game is built for the Classic Controller as well as the Remote: two sticks, triggers, buttons. No motion to synthesise (unlike DQS's sword) |
| **The engine was cross-platform** | Xbox, PC and PSP paths are still named in it (`PcRawMouseMove`, `PcSetControlBinding`); the controller layer is an abstraction (8 pads, sticks as floats) the port can feed |

| Harder | Why | Answer |
|---|---|---|
| **Size** | 1.42 M words, 19 421 functions, 9 491 indirect jumps (Victorious's size, DQS ×2.6) | already recompiled and linked; the dispatch table catches the rest |
| **Controllers arrive by event** | the game learns of a Remote and its extension from KPAD's connect callback, which the runtime never calls; a bare Remote is refused | wiikit's WPAD gains connection events and a Classic Controller (`08-input.md`) |
| **The renderer** | HVS's engine is the Wii's showcase of TEV: normal and bump maps, specular, reflections, bloom, depth of field (`DepthOfFieldRampAdvanced`), depth fog, HDR particles, "patched water" | the FIFO renderer grows what is missing, each feature reported on first use; Dolphin frame by frame |
| **Online everywhere** | DWC (Wi-Fi Connection), GameSpy, Quazal Net-Z, NHTTP, SSL, WiiConnect24, Wii Speak; the servers are gone; DWC has a fatal-error screen that loops forever | IOS answers "no network" as a console with no connection does; the online menus fail the way they fail on a Wii today. Not a goal: online play |
| **The aim** | a stick turns at a rate; a mouse moves by a distance | first the mouse as the right stick, then the mouse straight into the view angles (`08-input.md`) |

## Where to cut

Unchanged from Victorious (`../pc-victorious/docs/06-attack-plan.md`): the
CPU recompiled; graphics at the GX FIFO; audio at the AX micro-code; input
at WPAD/KPAD; files, saves and title at IOS's IPC registers; the OS
scheduler recompiled with only `OSLoadContext` replaced; VI presenting the
XFB; Bluetooth and low-level WPAD stubbed. From DQS: units from discovery,
names from `names.tsv`. New here:

* **Names**: the strat natives (`tools/natives.py`) and a word-for-word
  match against a symbolised ELF with the same SDK (`tools/elfmatch.py`),
  both headed for wiikit (`10-wiikit.md`).
* **Input**: the Classic Controller as the port's pad (not the Remote), and
  a second, later cut above KPAD for the mouse: the engine's own controller
  state (8 × 0x28 bytes, sticks as floats) or the player's view angles.
* **Network**: `/dev/net/*` answered as a console never set up for the
  Internet (NCD: no configuration; KD: WiiConnect24 off), so the game takes
  its offline paths.

## Phases

| # | Phase | Checkable milestone |
|---|---|---|
| 0 | **Analysis** ✅ | disc, executable, libraries, controls, route (session 1) |
| 1 | **Code map** ✅ (session 1) | 4 846 names; every hook target linked here resolved (`KPADReadEx`, `KPADInitEx` new) |
| 2 | **Recompiler** ✅ compiles and links (session 1); the self-test to port | 19 421 units, 173 files, no error; the native self-test through `names.tsv` |
| 3 | **Boot** 🟡 to the legal notices (session 1) | `__start` to the main loop ✅; the black screen after the notices; the Bink logos; the title |
| 4 | **Input foundations** 🟡 experiment (session 1) | the Classic Controller connected as the game expects; "controller required" gone; the title and the menus driven from the keyboard |
| 5 | **Graphics** | the front end (Scaleform over a 3D background, `99_99`), the Bink movies, the first level (the oil rig, `01_xx`); TEV breadth, indirect water, fog, DOF, bloom against Dolphin |
| 6 | **Audio** | the engine's AAL on AX: effects and music from the WADs' RIFF/DSP-ADPCM, Bink's sound, Pro Logic II |
| 7 | **Mouse and keyboard** | WASD on the left stick; the mouse on the aim (stick first, then the view angles directly); buttons on keys and mouse; a key file. Target: **the oil rig played with mouse and keyboard** |
| 8 | **PC finish** | 16:9 (native), resolution, saves in the NAND, English/French/Spanish, the online menus failing cleanly, local split-screen on SDL gamepads |

Phases 4–7 interleave once the title is up, as they did in Victorious and
DQS. Estimate: **eight to ten sessions** to the campaign played with mouse
and keyboard. The weight is in phase 5 (the renderer) and in the black
screen of phase 3, whose cause is not yet known.

## Known risks

* **The black screen after the legal notices** (`05-open-questions.md`):
  the main loop runs (400 frames a second, no frame limiter), no file is
  read after the Home Button's, the Bink logos are never opened. Suspects,
  in order: controllers (the game never polls them in that phase, even with
  a connection announced), WiiConnect24 (`/dev/net/kd` missing), the save
  check (the NAND is never touched). Dolphin shows what should come next.
* **Renderer breadth.** Conduit 2 pushes the TEV harder than anything
  wiikit has drawn: expect indirect textures for water and refraction, many
  TEV stages, EFB copies for bloom and DOF (146 000 already in the boot),
  Z textures. Dolphin frame dumps side by side, one effect at a time.
* **Controller events.** The runtime's WPAD is poll-only; this game (and
  likely others of 2010) wants callbacks. Calling guest callbacks from the
  host must happen on the game's thread at a safe moment (announcing it
  inside `KPADSetConnectCallback` crashed).
* **Mouse aim.** Through a stick it inherits the game's dead zone,
  acceleration and turn-rate cap. The direct way needs the player's view
  code, which may live in a strat (bytecode) rather than in C++.
* **The strat VM.** Game logic is bytecode (`.SVM` inside the `.gcs`
  WADs), interpreted by recompiled C++: debuggable only through the
  natives it calls, unless the VM is mapped.
* **Online paths.** DWC's fatal-error screen loops forever by design; the
  network must fail in the ways the game tolerates.
* **Single precision**, as in Victorious: physics and aim could notice.
  Differential runs against Dolphin.

## What is never distributed

Documentation, tools, and wiikit's recompiler and runtime. Generated C++,
game data and anything extracted stay local, produced from one's own disc.
