# TODO — session 2

Phase 3 to the title, and phase 4's foundations: the game boots and draws
its first screens (session 1), then sits on black at 400 frames a second.

1. **Dolphin as the reference, first.** Boot the disc in Dolphin with an
   emulated Classic Controller and capture what follows the legal notices
   (as DQS's capture script does), and the controls screen: the Classic
   layout (`05-open-questions.md` 5).
2. **The black screen** (`05-open-questions.md` 1): `--watch` on the main
   loop's state; the natives called per frame (a counting hook on the
   strat tables would show what the front-end script is doing); then test
   the suspects one at a time:
   * controllers: finish the Classic experiment (`08-input.md`): read the
     connect callback's channel-0 path (80015C80…) and the manager's update
     (8010E210) for what gates `KPADReadEx`;
   * WiiConnect24: answer `/dev/net/kd/request` and `/dev/net/kd/time` as
     a console with WiiConnect24 off (wiikit's IOS);
   * the GameCube pad (`05-open-questions.md` 3), if the Classic resists.
3. **The Classic Controller in wiikit** once the event order is known:
   KPAD's Ex status, connect and extension callbacks from the runtime, the
   key file's Classic buttons and sticks. Check on Victorious and DQS; then
   the sister ports' docs and submodules, as in session 1.
4. **The title and the menus** (Scaleform over `99_99`), driven from the
   keyboard: the first Bink movie (Logo_HVS_16x9.bik).
5. Port Victorious's native self-test through `names.tsv` (phase 2's
   remaining check).
6. `tools/elfmatch.py` and `tools/natives.py` toward wiikit (as `sig`),
   once a second game needs them.

Build (from the root, MSYS2's clang and ninja on PATH):

    python -m wiikit.disc "Conduit 2 (USA) (En,Fr,Es).wbfs" --extract build/extract
    python ../pc-dragonquestswords/tools/sigmatch.py build/extract/sys/main.dol \
        --dsy <Dolphin>/Sys/totaldb.dsy --elf <Victorious ELF> --out build/sig_guess.tsv
    python tools/elfmatch.py build/extract/sys/main.dol <Victorious ELF> > build/elf_names.tsv
    python tools/names.py build/extract/sys/main.dol
    python -m wiikit.recomp build/extract/sys/main.dol --out build/recomp \
        --symbols build/names.tsv --hooks tools/conduit2-hooks.txt
    cmake -S build/recomp -B build/recomp-build -G Ninja -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE=-O1 \
        -DWIIKIT_EXTRA=$PWD/tools/conduit2.cmake
    ninja -C build/recomp-build
    build/recomp-build/wiiboot build/extract --symbols build/names.tsv

Housekeeping: wiikit `1168fd6` is committed locally, not pushed (up to
`5b66083` is on GitHub; `93cfa20` was local already). Victorious `0bc90fa`
and DQS `bcbd779` bump their submodules, local. Push when the user says so.
