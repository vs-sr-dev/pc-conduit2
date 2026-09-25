# TODO — session 4

The game plays to its first level with WASD and the mouse, with a gamepad,
or with both at once, and in split-screen (session 3). The Classic
Controller and gamepads are wiikit's now (`5a004bf`, `b9db60f`), ready for
the next port (Arc Rise Fantasia, pad-first), which may well come before
this session. Next here: fog and smoothness, then the mouse made proper.

1. **Fog** (`gx: fog (not drawn), type 2`): wiikit's shader generator, with
   Dolphin's frame beside. DQS needs it too.
2. **Stutters on first sight** of something new: count programs linked per
   frame (`WIIKIT_PERF`); a program cache on disk (GL program binaries), or
   compiling the shaders a WAD needs while it loads. Game-agnostic: wiikit.
3. **Mouse look without the stick's cap**: find where the player's yaw and
   pitch take the right stick (`strat_ControllerAnalogRX/RY` callers, the
   player strat's turning natives), add the mouse's motion there, degrees
   per count. `WIIKIT_ICALLS` on a frame in play shows the natives the
   player strat calls. Then `mouse_on_right_stick` (`tools/conduit2.cpp`)
   goes.
4. **The Classic layout and remapping**: the game's controls screen (or its
   options) in Dolphin with the Classic profile; the key file's defaults
   follow it. The user wants a remapping of their own later (pad buttons
   too: today the pad's map is fixed but for `Face Buttons`).
5. **A pad unplugged and plugged back mid-game**, not yet tried: channel 2
   in split-screen should be told of the disconnection (the game's
   "reconnect" notice), and the pad come back on the same channel.
6. The black screen seen once in session 2 (after a save existed, with
   audio), never again since: watch for it; if it returns,
   `CONDUIT2_TRACE=1` shows the streamer's state.

Build as in session 1 (`00-sessions.md`), with the port's hooks:

    python tools/names.py build/extract/sys/main.dol
    python -m wiikit.recomp build/extract/sys/main.dol --out build/recomp \
        --symbols build/names.tsv --hooks tools/conduit2-hooks.txt
    ninja -C build/recomp-build
    build/recomp-build/wiiboot build/extract --symbols build/names.tsv [--input auto|pad|keyboard]

Controls: a gamepad (triggers ZL/ZR, shoulders L/R, Start +, Back -, guide
Home; face buttons by position, the right one A), and the key file's
`[Classic Controller]` (`build/keys.txt` has none: wiikit's defaults, WASD
the left stick, mouse left ZR, right ZL, Enter/Space A, Backspace or C B,
R X, F Y, E R, Left Shift L, Tab +, Q -, H Home, arrows the d-pad; the
mouse's motion the right stick). Esc the pause box (frees the mouse), F12
a GX trace. Split-screen: `--input keyboard` puts the pads from channel 2.

Housekeeping: wiikit is pushed up to `cb99bf8`; `5a004bf` and `b9db60f`
are local (audited: no private port named), for the user to push. The
ports' own commits are local: Conduit 2's whole history; Victorious up to
`86a676f`; DQS up to `2bb90f2`.
