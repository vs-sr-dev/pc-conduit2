# Open questions

1. ~~What the black screen after the legal notices waits for.~~ An IOS
   race (session 2, `00-sessions.md`); kept for the record: The main
   loop (8005E87C) runs at ~400 frames a second without its frame limiter,
   draws black, reads no file after the Home Button's, never opens the
   Bink logos. A network thread (8012D7B4) polls with `OSYieldThread`.
   Suspects: the controllers (never polled in that phase), WiiConnect24
   (`/dev/net/kd/request` and `/dev/net/kd/time` refused by IOS), the save
   check (the NAND is never touched). First step: what Dolphin shows after
   the notices, and a `--watch` with the main loop's state.
2. ~~How a channel becomes "connected" for the game.~~ The connect callback
   from `GXDrawDone` is enough (session 2). Kept for the record: The connect
   callback (800158F4) is called for channel 0 with the extension
   callback after it, and still nothing polls (`08-input.md`). What does the
   channel-0 path (80015C80…) set, and what does the manager's update
   (8010E210) test?
3. **Is the GameCube pad enabled in retail?** The input module calls PAD;
   `USE_GCN_CONTROLLER` is only in the development `game.txt`. If it is,
   SI is a simpler cut than Bluetooth events.
4. **Is the configuration file read at all?** `cdgame.txt` is named in the
   executable but never opened from the disc (replacing it with `game.txt`
   changed nothing). Compiled-in defaults, a host-only path (`GCNFile`), or
   read later? If the switches can be reached (in memory, by the
   parser's defaults), `LEVEL_SELECT`, `DEBUG_CHEATS`,
   `LOAD_TO_START_LEVEL` are a debug menu for the port.
5. **The Classic layout**: which button does what in play (fire, aim,
   jump, grenade, melee, reload, the ASE); read from the controls screen in
   Dolphin.
6. **Where the player's view takes the right stick**: in a strat
   (bytecode, through `ControllerAnalogRX/RY` and turning natives) or in
   C++. Decides how the mouse gets 1:1 aim.
7. **The strat VM**: interpreted bytecode (`.SVM`)? Where is the
   interpreter, how are natives called (the tables give the natives, not
   the call site)? A trace of natives per frame would be a cheap window on
   the game logic.
8. **The WAD container**: `BGIB` header, then an LZ with a flag byte every
   eight items. Only needed for tools (a WAD browser, texture dumps), not
   for the port.
9. **Frame rate**: 30 or 60 in play? The limiter (8005F284) divides by an
   fps value at `[r13-0x4D0C]`.
10. **Split-screen**: up to four local players; how many Classic
    Controllers the manager takes, and whether SDL gamepads can feed
    channels 1–3.
11. **The purple tones** where the game's lights are warm yellow (session
    2): what draws them, against Dolphin (`07-next-session.md` 1).
12. **The black screen seen once more** in session 2, with a save present
    and audio on; six runs after it went through. A second race, or the
    first one's margin?
