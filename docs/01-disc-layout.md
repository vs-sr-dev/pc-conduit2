# The disc

`Conduit 2 (USA) (En,Fr,Es).wbfs`: game ID **SC2E8P** (NTSC-U, Sega), title
"Conduit 2", one DATA partition of 4 425 MB, title ID `0001000053433245`.
Extracted by `wiikit.disc` at the first try: **821 files, 3 990 MB**.

    python -m wiikit.disc "Conduit 2 (USA) (En,Fr,Es).wbfs" --info
    python -m wiikit.disc "Conduit 2 (USA) (En,Fr,Es).wbfs" --extract build/extract

## What is on it

| What | Files | Size | Notes |
|---|---|---|---|
| `XX_YY.gcs` | 495 | 3 067 MB | "Strat WADs": compiled scripts and sound (RIFF/WAV inside), one per WAD |
| `XX_YY.gcm` | 266 | 422 MB | map and art WADs: geometry, textures (`Maps\01_01.amf`) |
| `Movies/*.bik` | 7 | 176 MB | Bink: HVS and publisher logos (16:9 and 4:3), prologue, suit-up, epilogue (16:9 only) |
| `levels.txt` | 1 | 21 KB | the WAD list: id, author, name, build date |
| `cdgame.txt`, `game.txt` | 2 | 5 KB | engine configuration: retail and development (see below) |
| `HomeButton2/` | 16 | 4.5 MB | the SDK's Home Button menu, nine languages |
| `sys/mpls_movie/` | 30 | 137 MB | Nintendo's MotionPlus instructional video: its own `player.dol` and seven RELs, standard, not the game |
| `opening.bnr`, `savebanner.tpl`, `saveicons.tpl` | 3 | | banner and save icons |

No BRRES, no BRSTM, no NW4R layouts outside the SDK's own packages: the game
runs on its studio's engine, not on Nintendo's middleware.

## WADs

Everything the game loads comes in WADs named `XX_YY`, listed in
`levels.txt` (265 entries, all by one author, all built on "Mar 06"). A
level is several WADs, loaded and suspended by the scripts
(`SetStartWad`, `SetSharedWad`, `SuspendWad`). Each WAD is a pair: `.gcm`
(map, art) and `.gcs` (scripts, sound), not every WAD has both.

| Group | WADs | First … last name |
|---|---|---|
| 01 | 43 | Helipad … textureLibraryOilPlatform (the oil rig) |
| 02 | 1 | Crane Platform |
| 03 | 24 | Bunker … Courtyard Upper Floor |
| 04 | 26 | C2 Cyberia Prison start … C2 Cyberia Canyon End |
| 05 | 18 | China … textureLibraryChina |
| 06–18 | 1–19 each | the hub (Hub_DCTime, The Hub, Last_Hub), Atlantean Graveyard, Agartha, Cliff Dwellings, Crash Site, Fountain |
| 30, 31, 34 | 11 | Trenches … Frozen |
| 40, 41, 44 | 9 | Siberia Intro, DropShip, Li Boss Fight |
| 50 | 19 | Pistol … War Gauntlet (weapons) |
| 60 | 36 | Globe … Tomb |
| 70, 71 | 25 | C1 Pentagon … Skin Select Preview, Fountain (multiplayer, with the first game's maps) |
| 99 | 4 | Atlantis Front End Background … Objective Manager (the front end; `99_99` is the first WAD read) |

The largest are the sound-heavy `.gcs` of whole chapters: `03_99.gcs`
208 MB, `70_60.gcs` 200 MB, `04_99.gcs` 169 MB.

### The container (first look)

Both kinds start with the same header: magic `BGIB` ("BIGB" byte-swapped),
0x1F0 bytes, the WAD's name at 0x11 ("Helipad", "<Strat Wad>"), the author
at 0x51, and in a `.gcm` the converter's command line
(`-w-wii -quiet -disableerrors -nodefaultmaplights ... Maps\01_01.amf`).
Past the header the data is compressed: an LZ with a flag byte every eight
items, the literals readable between them (`RIFF…WAVE`, `name`,
`Cinema1_SFXSLUG_11_1279.wav`, `fmt `, `strm`). Not decoded yet; a
recompiled port reads its WADs with the game's own code and never needs to.

## The configuration files

The engine reads a text configuration at boot. The executable names only
`cdgame.txt`, the retail one. `game.txt`, left on the disc beside it, is a
developer's: the same keys with the debug switches on.

| Key | `cdgame.txt` | `game.txt` |
|---|---|---|
| `LEVEL_SELECT` | OFF | ON |
| `DEBUG_CHEATS` | OFF | ON |
| `DEBUG_TEXT`, `DEBUG_SCREENPRINTS`, `DEBUG_STRAT_PRINTS` | OFF | ON |
| `SKIP_MOVIES` | OFF | ON |
| `START_LEVEL` (with `LOAD_TO_START_LEVEL`) | 03_01 | 40_03 |
| `USE_GCN_CONTROLLER`, `DEBUG_MP_LEVEL_SELECT`, `EDIT_COLOR_CURVES` | absent | ON / ON / OFF |

Both have `USE_CLASSIC_CONTROLLER ON`, `LOAD_TO_START_LEVEL OFF`,
`PERFORMANCE_METRICS 0`, `HEAPSIZE 32.0`, the heap sizes of the engine's
parts ("Diesel", Scaleform, "Quantum", audio) and online keys (`DWC_*`,
`AUTO_HOST`, `AUTO_CONNECT`). Keys left from other platforms: `PSP`,
`KEYCODE … // Region Code from Sony`.

For the port this is a free debug menu: `game.txt` read in place of
`cdgame.txt` (one string in the executable, or the file swapped in the
extract) should give the level select, cheats and a straight start into any
WAD, without touching code. To be tried (`05-open-questions.md`).
