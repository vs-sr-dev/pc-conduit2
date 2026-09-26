# This port and wiikit

This port takes [wiikit](https://github.com/vs-sr-dev/wiikit) as a
submodule at `wiikit/`, like pc-victorious and pc-dragonquestswords. It is
its third user, and the second with a stripped executable. wiikit is public
and does not name this port; changes made for it are described by what
they do, and checked on every port before they go in.

## What this port used as it is (session 1)

`wiikit.disc` (the WBFS at the first try: 821 files), `dol`, `ppc`
(14 undecoded words in 1.42 M), the recompiler with discovery (19 421
units, 0 gaps, 175 switch tables; no change needed at three times DQS's
size), and the whole runtime: to the Wii Strap, the controller message, the
legal notices, AX running, 16:9 from SYSCONF.

## What it gave wiikit

| Commit | Change | Checked on |
|---|---|---|
| `a07e7ab` | `KPADInitEx` and `KPADReadEx` hooked (the 2010 SDK's Ex forms; `KPADRead` is not linked here) | Victorious: one hook slot more in its C++ (it calls `KPADInitEx`), self-test 15/15, booted to the Auditions episode as before. DQS: C++ unchanged, booted to its menus |
| `a66e681` | the PI FIFO's pointers keep MEM2 (bits 0-28) and wrap only at the end; `mtspr WPAR` empties the gather buffer; IOS replies no sooner than 50 us after the request, or at once when the CPU idles; F12 GX trace; `WIIKIT_ICALLS`; opt-in relative mouse; `--mmio-log` logs failed opens and every ioctl | Victorious: C++ changed at one `mtspr WPAR`, self-test 15/15, frame for frame to its first episode. DQS: one `mtspr WPAR`, to its menus |
| `5a004bf` | **the Classic Controller**: `wpad_set_classic`, KPAD's Classic status on every channel that has one; SDL3 gamepads as Classic Controllers (a channel each as plugged in; triggers ZL/ZR, face buttons by position or label, dead zone); channel 1 the key file's `[Classic Controller]` keys and mouse buttons merged with the first pad; `--input auto\|pad\|keyboard`; the connect and extension callbacks called after `__VIRetraceHandler` or at the next read (Classic games only); the Remote's motor on the pad; `wpad_set_classic_filter` for a port's mouse | this game played by the user with an Xbox One pad and keys+mouse at once. Victorious: one hook slot more (`__VIRetraceHandler`), self-test 15/15, to the Auditions episode as before. DQS: C++ unchanged, to its menus (its own connect callback, when called, started WPAD's sampling and crashed: hence Classic games only) |
| `b9db60f` | the input modes renamed `INPUT_MODE_*`: `windows.h`'s `INPUT_KEYBOARD` made `--input keyboard` mean `pad` | split-screen played (keys+mouse on channel 1, the pad on channel 2); Victorious 15/15 and its episode, DQS its menus |

The sister ports moved their submodule each time (Victorious `ce21ffe`,
`3df589a`, `d8ce10b`, `b492489`, `19997a8`; DQS `e04ad30`, `39ea570`,
`aa45577`, `cde8c05`, `b7bcbb3`).

## What it will give wiikit

| Addition | Layer | Why it is not game knowledge |
|---|---|---|
| `elfmatch`: a stripped DOL named word for word from a symbolised ELF with the same library builds (relocations masked, unique both ways, small functions anchored by their branch targets) | 3 | any game on an SDK build a symbolised executable shares; beats signature hashing when it applies (`tools/elfmatch.py`) |
| string tables of `{name, fn}` pairs as a name source | 3 | script engines and command tables are common; the prefix and pairing are the only game detail (`tools/natives.py`) |
| the network as a console never set up: `/dev/net/ncd`, `/dev/net/kd`, `/dev/net/ip/top` answering "not configured" | 5 | every game with WiiConnect24 or Wi-Fi Connection |
| a program cache (no stutter when a shader is first needed) | 5 | any game |
| whatever the renderer lacks for HVS's TEV (indirect water, fog, Z textures, EFB formats) | 5 | any 3D game |

The strat VM, WAD formats and the mouse-look hook into the player's view
stay here.
