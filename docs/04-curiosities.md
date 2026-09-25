# What the disc reveals

* **A developer's configuration left on the disc.** `game.txt` sits next
  to the retail `cdgame.txt` with `LEVEL_SELECT ON`, `DEBUG_CHEATS ON`,
  `DEBUG_FLY` (a fly-through mode, off), `LOAD_TO_START_LEVEL` with
  `START_LEVEL 40_03` (Siberia's intro), `AUTO_MONKEY` ("automatically run
  the controller": a random-input tester), `LEVEL_WALK`, `AUTO_HOST` and
  `AUTO_CONNECT` for network tests, and `USE_GCN_CONTROLLER ON`.
* **The engine came from elsewhere.** Keys like `PSP ON`, `KEYCODE BA //
  Region Code from Sony`, `PRODUCT_NUMBER // Product Code from Sony`; natives
  `XbBindController`, `XbRebootToDash`, `PcChangeToResolution`,
  `PcSetMultisampleQuality`, stubbed; `GCNFile`, `GCNSkin.cpp`,
  `GCNcControllerManager.cpp`: High Voltage's engine ran on PSP, Xbox, PC
  and GameCube before the Wii.
* **2 578 script natives** in the engine's tables, named with an `ass_` prefix,
  among them a controller "monkey" (`vEnableControllerMonkey`), network
  logging (`NL_OpenNetworkLogging`), Wii Speak and MotionPlus calibration.
* **One author, one day**: all 265 WADs in `levels.txt` are by
  "JohnSanderson", built on "Mar 06".
* **The first game inside the second.** WAD groups 70 and 71 start with
  "C1 Pentagon": multiplayer maps from The Conduit, next to a "Skin Select
  Preview".
* **Texture libraries per world**: `textureLibraryOilPlatform`,
  `textureLibraryChina` close their groups.
* **The map compiler's command line** is kept in each `.gcm`:
  `-w-wii -quiet -disableerrors -nodefaultmaplights
  -noautomaticshadowreceivers -useindexedtrilists -maxmodelsize 10.0
  -maxedgelength 25.0 Maps\01_01.amf`.
* **The legal screen** credits Scaleform, Dolby Pro Logic II, Bink and
  Quazal, and "under license from Raptor Game LLC".
