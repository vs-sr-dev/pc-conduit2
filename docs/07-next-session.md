# TODO — session 3

The game plays to its first level with WASD and the mouse (session 2).
Next: what it looks like, how smoothly, and the input made proper.

1. **The purple tones** (the user: warm yellow lights come out purple,
   "almost CGA"). Take the same scene in Dolphin (`tools/dolphin.py`, or F9
   there) and in the port (F12 for its GX commands, `--dump` for the
   picture); compare draw by draw (`WIIKIT_DRAWLOG` + `WIIKIT_EFBDUMP` on
   the traced frame). Excluded so far: IA8 palettes, EFB copies' channels
   (R8, GB8), the indirect coordinates' channel order. Suspects: fog type 2
   (not drawn; the log says so), TEV colour swaps, a colour-curve pass.
2. **Fog** (`gx: fog (not drawn), type 2`): wiikit's shader generator, with
   Dolphin's frame beside. DQS needs it too.
3. **Stutters on first sight** of something new: count programs linked per
   frame (`WIIKIT_PERF`); a program cache on disk (GL program binaries), or
   compiling the shaders a WAD needs while it loads. Game-agnostic: wiikit.
4. **The Classic Controller into wiikit** (`08-input.md`): KPAD's Ex status,
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

Housekeeping: wiikit `1168fd6` and `2733ba3` are local, not pushed;
Victorious `0bc90fa`, `ca1a9b5` and DQS `bcbd779`, `84426ea` bump their
submodules, local. Push when the user says so.
