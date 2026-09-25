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
* **Booted** after one fix, in wiikit (`1168fd6`): `KPADInitEx` ran the
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
* **wiikit** (`10-wiikit.md`): `1168fd6`, checked on both other ports
  (Victorious self-test 15/15 and booted to its first episode; DQS
  unchanged, booted to its menus); their submodules moved on.
