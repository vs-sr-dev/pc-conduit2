# Session log

## Session 1 — analysis, the plan, and first light

Goal: understand the disc and the code, see how far wiikit carries a third
game, settle the controls, choose a route. It went past the plan: the game
boots and draws its first screens.

Results:

* **The disc** (`01-disc-layout.md`): SC2E8P, the WBFS extracted by
  `wiikit.disc` unchanged, 821 files, 3.99 GB. Everything the game loads is
  in 265 WADs (`XX_YY.gcm` art, `XX_YY.gcs` scripts and sound, a `BGIB`
  header and an LZ), seven Bink movies, the Home Button menu, Nintendo's
  MotionPlus video. A developer's `game.txt` sits beside the retail
  `cdgame.txt` (level select, cheats, start level), but the game opens
  neither from the disc.
* **The executable** (`03-executable.md`): stripped, 1.42 M words, no RELs.
  **The RVL SDK is byte for byte Victorious's** (every library the Aug 23
  2010 build). High Voltage's engine, cross-platform (PC, Xbox, PSP
  leftovers), driven by a script language, strats; Scaleform GFx, Bink,
  Quazal, DWC/GameSpy, Wii Speak.
* **Names**: 4 846, from three sources. The strat engine's tables
  (`tools/natives.py`) name **2 575** engine functions by themselves
  (`ControllerAnalogRX`, `AllowClassicInput`, `PcRawMouseMove`…). A new
  matcher (`tools/elfmatch.py`) finds Victorious's ELF functions word for
  word in this DOL: **2 217** SDK, MSL, Bluetooth, HBM functions in 20 s.
  `KPADRead` is not linked; the game reads with `KPADReadEx`.
* **Recompiled, compiled, linked** at the first try: 19 421 units, no
  gaps, 175 switch tables, 173 files of C++, no error.
* **Booted** after one fix, in wiikit (`a07e7ab`): `KPADInitEx` ran the
  real KPAD into an unstarted WPAD. With `KPADInitEx` and `KPADReadEx`
  hooked, the game runs from `__start` through `OSInit`, the static
  constructors, AX, to its main loop and draws, in 16:9: the Wii Strap
  screen, **"A Nunchuk or Classic Controller is required to play this
  game."**, the legal notices (Scaleform, Dolby, Bink, Quazal). Then black,
  the main loop at ~400 fps (the user saw it too), no more files read.
* **Controls** (`08-input.md`): the game takes the Classic Controller;
  the engine keeps 8 abstract pads with float sticks. It learns of
  controllers only from KPAD's connect callback, which the runtime never
  calls. An experiment in the port's layer (a Classic Controller reported,
  the connect and extension callbacks called from `GXDrawDone`) runs but
  does not yet satisfy the game. The GameCube pad is live code too.
* **The plan** (`06-attack-plan.md`): Victorious's route; eight phases,
  eight to ten sessions to the campaign played with mouse and keyboard;
  the mouse first as the right stick, then straight into the view.
* **wiikit** (`10-wiikit.md`): `a07e7ab`, checked on both other ports
  (Victorious self-test 15/15 and booted to its first episode; DQS
  unchanged, booted to its menus); their submodules moved on.

## Session 2 — the game plays, with WASD and the mouse

Goal (`07-next-session.md` of session 1): the black screen after the legal
notices, then the title. It went to the first level, played by the user
with the keyboard and mouse, characters whole.

Results:

* **Dolphin as the reference** (`tools/dolphin.py`: starts the disc,
  screenshots the render window on a timeline, presses keys; a Classic
  profile loaded only for SC2E8P). After the legal notices: the HVS and
  SEGA logos (Bink), then the title "Press A to Continue" over a 3D
  canyon. "A Nunchuk or Classic Controller is required" shows in Dolphin
  too: a fixed notice, not a complaint.
* **The black screen was a race in IOS** (wiikit `a66e681`). Found through
  what the front-end script waits on (`WIIKIT_ICALLS`, new: indirect calls
  counted by target, which names the strat natives it calls every frame:
  `StreamingIdle`, `WadLoaded`, `StreamingNANDResult`), then the WAD
  streamer's state (`cWadStreamer_*`, 6 = waiting), then its NAND reader:
  before a WAD is read from the disc, the game looks for a **downloaded
  patch** of it in the NAND (`data/patch/99_70.gcs`). The runtime answered
  `NANDOpenAsync` inside the call: the reader's completion (-12, no such
  file) ran before the reader marked itself pending, which then
  overwrote it, and waited forever. IOS replies now come at least 50 us
  after the request, or at once when the processor idles (synchronous
  calls cost nothing). The game falls back to the disc and goes on: logos,
  title, menus, profile, the opening movie, the oil rig (`01_xx`).
  The game also pre-allocates `data/patch/dummy`…`dummy6` in the NAND.
* **The user plays it.** New Game, a profile, the prologue movie, the
  first level: HUD, weapon, rain, the Glomar platform.
* **Input** (`08-input.md`): the Classic Controller of session 1's
  experiment, now read straight from SDL in the port's layer: WASD the
  left stick, the mouse's motion the right stick (`CONDUIT2_MOUSE` for the
  gain), the mouse buttons ZR and ZL, Enter/Space A, Backspace B. The
  mouse is captured (wiikit's new opt-in relative mode): in a pointer game
  it never mattered. Mouse look through the stick works; the stick caps
  the turn rate.
* **Characters were half missing, then exploding** — three wiikit bugs
  in how the GP FIFO and the write-gather pipe work, none of which
  Victorious or DQS had reached (`a66e681`). The engine skins models on
  the CPU (`GCNSkin.cpp`, a table of five paired-single routines,
  805A6190) straight into the gather pipe redirected to a buffer in MEM2
  (`GXRedirectWriteGatherPipe`), and records display lists in MEM2:
  1. the PI FIFO pointers were masked to 26 bits: MEM2 writes landed in
     MEM1 and display lists measured 0xF000xxxx bytes, skipped (half the
     body missing, the hero's hand too);
  2. the pointer wrapped when past the end (the redirect sets end
     0x04000000, below any MEM2 buffer): every byte restarted at 0;
  3. `mtspr WPAR` did not empty the gather buffer: the redirect's padding
     remainder landed first in the vertex buffer and shifted every vertex
     (triangles "exploding from the centre of the screen").
  Found with F12 (new: the next frame's GX commands to a file) pressed by
  the user in front of a character. Confirmed whole by the user.
* wiikit `a66e681` and `cb99bf8` checked on both ports (Victorious self-test 15 of 15
  and frame for frame to its first episode; DQS to its menus); their
  submodules moved on (Victorious `ca1a9b5`, `befa664`; DQS `84426ea`,
  `d6cb24b`).

* **Purple where the lights are yellow** (the user's screenshots: yellow
  pipes magenta, the sky unchanged: green and blue swapped; right again
  once the pause menu is *opened*). A differential trace (F12 before and
  after the menu, standing still) showed one pass only in the purple
  frame: a full-screen colour grading of 8 TEV stages and 3 indirect
  stages, each channel looked up in a curve texture through an indirect
  offset, masked by a konst colour (K0 red, K1 green, K2 blue). Red comes
  from an R8 copy; green and blue from one GB8 copy read as IA8, green as
  its intensity and blue as its alpha. wiikit's copy shader had RG8 and
  GB8 the other way round (wiikit `cb99bf8`). The menu "fixed" it by
  switching the grading off. Confirmed by the user: right everywhere,
  menus and game; the game's grading now applies from the start.

Left: fog type 2 not drawn; stutters when something new first appears (shader
programs compiled on first use); the black screen seen once more in one
session and never again in six runs.
