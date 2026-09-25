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
| `1168fd6` | `KPADInitEx` and `KPADReadEx` hooked (the 2010 SDK's Ex forms; `KPADRead` is not linked here) | Victorious: one hook slot more in its C++ (it calls `KPADInitEx`), self-test 15/15, booted to the Auditions episode as before. DQS: C++ unchanged, booted to its menus |
| `2733ba3` | the PI FIFO's pointers keep MEM2 (bits 0-28) and wrap only at the end; `mtspr WPAR` empties the gather buffer; IOS replies no sooner than 50 us after the request, or at once when the CPU idles; F12 GX trace; `WIIKIT_ICALLS`; opt-in relative mouse; `--mmio-log` logs failed opens and every ioctl | Victorious: C++ changed at one `mtspr WPAR`, self-test 15/15, frame for frame to its first episode. DQS: one `mtspr WPAR`, to its menus |

The sister ports moved their submodule each time (Victorious `0bc90fa`,
`ca1a9b5`, `befa664`; DQS `bcbd779`, `84426ea`, `d6cb24b`; local commits).

## What it will give wiikit

| Addition | Layer | Why it is not game knowledge |
|---|---|---|
| `elfmatch`: a stripped DOL named word for word from a symbolised ELF with the same library builds (relocations masked, unique both ways, small functions anchored by their branch targets) | 3 | any game on an SDK build a symbolised executable shares; beats signature hashing when it applies (`tools/elfmatch.py`) |
| string tables of `{name, fn}` pairs as a name source | 3 | script engines and command tables are common; the prefix and pairing are the only game detail (`tools/natives.py`) |
| **the Classic Controller**: extension type and status in KPAD, the key file's Classic buttons and sticks, SDL gamepads | 5 | every Wii game with Classic support |
| **controller events**: KPAD's connect callback and WPAD's extension callback called on the game's thread at a safe moment (done in the port's layer, from `GXDrawDone`, session 2) | 5 | games of 2010 wait for them (this one never polls without them) |
| the network as a console never set up: `/dev/net/ncd`, `/dev/net/kd`, `/dev/net/ip/top` answering "not configured" | 5 | every game with WiiConnect24 or Wi-Fi Connection |
| the mouse as a stick (delta → deflection; the relative mouse is in `2733ba3`) | 5 | any twin-stick game |
| a program cache (no stutter when a shader is first needed) | 5 | any game |
| whatever the renderer lacks for HVS's TEV (indirect water, fog, Z textures, EFB formats) | 5 | any 3D game |

The strat VM, WAD formats and the mouse-look hook into the player's view
stay here.
