# Input: from the Classic Controller to mouse and keyboard

## What the game accepts

Conduit 2 plays with the Remote and Nunchuk (pointer aim, the first game's
scheme, with MotionPlus support) **or with the Classic Controller** (two
sticks; the first game had no Classic support). The port's first light
confirms it: with the runtime's bare Remote the game says

> A Nunchuk or Classic Controller is required to play this game.

In the code:

* The engine's controller layer is an abstraction over 8 pads: per pad
  0x28 bytes at `[r13-0x3414] + 0x3044`, sticks and triggers as floats,
  filled by the controller manager (8010DE40–8010E600) from `KPADReadEx`.
  The scripts read it through natives: `ControllerAnalogLX/LY/RX/RY`,
  `…LDist/LAngle/RDist/RAngle`, `…LTrigger/RTrigger`, `ControllerPad`,
  `ControllerPush/Pull`, `AllowClassicInput`, `AllowClassicDpad`.
* `USE_CLASSIC_CONTROLLER ON` in both configuration files.
* **The GameCube pad is live code**: the input module (80015xxx) calls
  the SDK's PAD library (read, `PADClampCircle`, `PADControlMotor`), and the
  development `game.txt` has `USE_GCN_CONTROLLER ON` (the retail file does
  not name it). Whether retail enables it is open; if it does, it is an
  even simpler cut (SI, no Bluetooth events).
* Leftovers of the PC version: natives `PcRawMouseMove`, `PcMousePos`,
  `PcSetControlBinding`, `PcEnterBindMode`… are stubs on the Wii (they print
  "only exists in P_TABLE as stub!").

## How the game finds a controller

Not by polling. For each channel the manager calls
`KPADSetConnectCallback(chan, 800158F4)` (from 8010DE68); on a real Wii,
KPAD calls it when a Remote connects, and the callback registers an
extension callback and initialises the channel's state. The game never
calls `KPADReadEx` or `WPADProbe` for a channel it was not told about.

wiikit's WPAD is poll-only: it accepts the callbacks and never calls them.
Session 1's experiment (`tools/conduit2.cpp`, `tools/conduit2-hooks.txt`):

1. `WPADProbe` and `KPADReadEx` report a Classic Controller (`dev_type` 2,
   `data_format` 8, `ex_status.cl` at 0x60 with buttons mapped from the
   Remote's keys);
2. the connect callback is called for channel 0 with `WPAD_ERR_NONE`, and
   the extension callback with `WPAD_DEV_CLASSIC`. Called inside
   `KPADSetConnectCallback` it crashed (the game's tables not yet set);
   called from `GXDrawDone` ten frames later it runs.

Session 1 thought this was not enough, because the message stayed. It
was: the message is a fixed notice of the boot (Dolphin shows it too with a
Classic Controller attached), and the game only seemed deaf because it was
stuck on a NAND race (`00-sessions.md`, session 2). With it fixed, the
user played the menus and the first level.

Session 2 feeds the Classic's status from SDL in the port's layer
(`classic_from_host`): buttons from keys and mouse buttons, the left stick
from WASD (0.707 on diagonals), the right stick from the mouse's motion per
sample times `CONDUIT2_MOUSE` (0.15), clamped to ±1; the cursor captured by
wiikit's relative mouse. Mouse look works; the stick's ±1 caps how fast the
view turns, which the direct way (below) removes.

## The scheme for the PC

The user's brief: the left stick on WASD, the right stick on the mouse.

1. **The Classic Controller in wiikit** (game-agnostic): extension type and
   status, connection and extension events, the key file gaining Classic
   buttons (`ZL`, `ZR`, `L`, `R`, `X`, `Y`, `A`, `B`, `+`, `-`, `Home`, the
   d-pad) and stick directions (`Left Stick Up = W` …), and SDL gamepads as
   real Classic Controllers (split-screen: up to four).
2. **WASD**: the left stick at full deflection in eight directions; a
   walk key (half deflection) if the game has a walk speed.
3. **The mouse on the aim, by stages**:
   * *as the right stick*: mouse movement per frame → stick deflection,
     with the game's curve measured and inverted (the native
     `ControllerAnalogRX` returns what the port writes at 0x3044+);
     quick, but it inherits the dead zone, acceleration ramps and the
     turn-rate cap;
   * *straight into the view*: find where the player's yaw and pitch take
     the right stick (the player strat's natives: `TurnLeft`, `SetYRot`,
     `ApplyAxisRot`…, or the C++ camera), and add the mouse there,
     degrees per count, no cap: real mouse-look. The target.
4. **Buttons**: fire and aim on the mouse buttons, the rest on keys, all in
   the key file; the mapping follows the game's own Classic layout, to be
   read from its controls screen in Dolphin.

The Remote-and-Nunchuk scheme (pointer = mouse, the game's own bounding-box
turning) comes for free from wiikit and can stay as an option, but it is not
the PC scheme.
