# TODO — session 3

The game plays to its first level with WASD and the mouse (session 2).
Its colours are right (the grading pass, wiikit `cb99bf8`). Next: the input
made proper (first: a fourth port, Arc Rise Fantasia, a JRPG, wants a pad),
then fog and smoothness.

1. **Gamepads and the Classic Controller in wiikit** (decided with the
   user): SDL3's `SDL_Gamepad` (XInput, DualShock/DualSense, Switch Pro:
   no new dependency) as the Classic Controller: sticks, d-pad, face
   buttons, shoulders and triggers; rumble (`WPADControlMotor` ->
   `SDL_RumbleGamepad`); pads on channels 1-4 as they are plugged in,
   raising KPAD's connect and extension events from the runtime; the key
   file gaining the Classic's buttons and sticks as the keyboard fallback;
   confirm/cancel positional (Nintendo) by default, by label as an option.
   The port's layer then keeps only the mouse's part. Test with a real pad
   in Conduit (and its split-screen); check on Victorious and DQS as ever.
   Details below (item 4) and in `08-input.md`.
2. **Fog** (`gx: fog (not drawn), type 2`): wiikit's shader generator, with
   Dolphin's frame beside. DQS needs it too.
3. **Stutters on first sight** of something new: count programs linked per
   frame (`WIIKIT_PERF`); a program cache on disk (GL program binaries), or
   compiling the shaders a WAD needs while it loads. Game-agnostic: wiikit.
4. **(Part of 1.) The Classic Controller into wiikit** (`08-input.md`): KPAD's Ex status,
   the connect and extension events from the runtime, key-file entries for
   the Classic's buttons and sticks, SDL gamepads as real Classic
   Controllers. Then the port's layer keeps only the mouse-to-view part.
5. **Mouse look without the stick's cap**: find where the player's yaw and
   pitch take the right stick (`strat_ControllerAnalogRX/RY` callers, the
   player strat's turning natives), add the mouse's motion there, degrees
   per count. `WIIKIT_ICALLS` on a frame in play shows the natives the
   player strat calls.
6. **The Classic layout**: the game's controls screen (or its options) in
   Dolphin with the Classic profile; the key map follows it.
7. The black screen seen once in session 2 (after a save existed, with
   audio), never again in six runs: watch for it; if it returns,
   `CONDUIT2_TRACE=1` shows the streamer's state.

Build as in session 1 (`00-sessions.md`), with the port's hooks:

    python tools/names.py build/extract/sys/main.dol
    python -m wiikit.recomp build/extract/sys/main.dol --out build/recomp \
        --symbols build/names.tsv --hooks tools/conduit2-hooks.txt
    ninja -C build/recomp-build
    build/recomp-build/wiiboot build/extract --symbols build/names.tsv

Keys (the port's layer, a guess until the controls screen is read): WASD
move, mouse look, left button ZR, right button ZL, Enter/Space A, Backspace
or C B, R X, F Y, E R, Left Shift L, Tab +, Q -, H Home, arrows the d-pad;
Esc the pause box (frees the mouse), F12 a GX trace.

Housekeeping: wiikit is pushed up to `cb99bf8` (its last three commits'
messages reworded before publishing so that no private port is named; the
trees unchanged). The ports' own commits are local: Conduit 2's whole
history; Victorious `0bc90fa`..`82901f9`; DQS `bcbd779`..`304d36f`.
