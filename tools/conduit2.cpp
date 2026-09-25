// Conduit 2 — the port's own layer over the wiikit runtime.
//
// Linked into wiiboot by conduit2.cmake. What belongs here is what only this
// game needs: the mouse on the view, the traces.
#include "rt.h"
#include "video.h"
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <cstring>

namespace {

// The game plays with the Classic Controller (docs/08-input.md): wiikit's,
// the keys of the key file's [Classic Controller] and the gamepads. It learns
// of controllers only from KPAD's connect callback, which wiikit calls. ("A
// Nunchuk or Classic Controller is required" is a fixed notice of the boot,
// shown in Dolphin too.) What stays here is the mouse: its motion on channel
// 1's right stick, added to the pad's, clamped to the stick's travel. Later,
// straight into the view (07-next-session.md).
float g_mouse_gain = 0.15f;                          // stick deflection per pixel of motion per sample (CONDUIT2_MOUSE)

void mouse_on_right_stick(int chan, ClassicState& s) {
    if (chan != 0) return;
    float dx = 0, dy = 0;
    video_take_mouse_motion(dx, dy);
    s.rx = std::clamp(s.rx + dx * g_mouse_gain, -1.0f, 1.0f);
    s.ry = std::clamp(s.ry - dy * g_mouse_gain, -1.0f, 1.0f);
}

// CONDUIT2_TRACE=1: strat natives logged, their first calls and then every
// change of what they return. A native is (r3: the strat, r4: its argument
// words, the result written over the first).
struct Traced { const char* name; PPCFunc orig; uint32_t calls, last_arg, last_res; };
Traced traced[] = {{"strat_SetStartWad"}, {"strat_SetStartWadI"}, {"strat_SetSharedWad"},
                   {"strat_SetSharedWadI"}, {"strat_WadLoaded"}, {"strat_StreamingIdle"},
                   {"strat_StreamWad"}, {"strat_StreamWad2"}, {"strat_StreamWadI"},
                   {"strat_SuspendWad"}, {"strat_StreamingNANDResult"}, {"strat_StreamWadNAND"},
                   {"strat_StreamWadNAND2"}, {"strat_StreamWadNANDI"}};

template <int I>
void trace(PPCContext& c) {
    Traced& t = traced[I];
    uint32_t args = c.r[4];
    uint32_t a0 = args ? ld32(args) : 0, a1 = args ? ld32(args + 4) : 0;
    t.orig(c);
    uint32_t res = args ? ld32(args) : 0;
    if (t.calls++ < 3 || res != t.last_res || a0 != t.last_arg)
        rt_log("strat: %s(%08X %08X) -> %08X  [call %u]", t.name, a0, a1, res, t.calls);
    t.last_arg = a0;
    t.last_res = res;
    if (!std::strcmp(t.name, "strat_StreamingIdle") && !res && t.calls % 1000 == 0) {
        uint32_t st = ld32(0x80664460 + 0x10);       // the WAD streamer
        rt_log("streamer %08X: state %u, df0 %08X, e00..e14 %08X %08X %08X %08X %08X %08X, dd0 %08X",
               st, ld32(st), ld32(st + 0xDF0), ld32(st + 0xE00), ld32(st + 0xE04), ld32(st + 0xE08),
               ld32(st + 0xE0C), ld32(st + 0xE10), ld32(st + 0xE14), ld32(st + 0xDD0));
        uint32_t rq = ld32(st + 0xDFC);
        if (rq) rt_log("  request %08X: \"%s\", patch %u, reader %08X (+0xC54 is %08X), dcc %08X",
                       rq, guest_cstr(rq + 0x10).c_str(), ld8(rq + 0x30), ld32(st + 0x20), st + 0xC54, ld32(st + 0xDCC));
        uint32_t vt = ld32(ld32(st + 0x20));
        rt_log("  reader vtable %08X: %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X", vt, ld32(vt), ld32(vt + 4),
               ld32(vt + 8), ld32(vt + 0xC), ld32(vt + 0x10), ld32(vt + 0x14), ld32(vt + 0x18), ld32(vt + 0x1C),
               ld32(vt + 0x20), ld32(vt + 0x24));
    }
}

template <int... I>
void install_traces(std::integer_sequence<int, I...>) {
    ((traced[I].orig = ppc_hook(traced[I].name, trace<I>)), ...);
}

PPCFunc nand_open_cb = nullptr, reader_done = nullptr, skin_stub = nullptr, skin_gqrs = nullptr;

void install() {
    if (const char* m = std::getenv("CONDUIT2_MOUSE")) g_mouse_gain = (float)std::atof(m);
    video_set_relative_mouse(true);
    if (std::getenv("CONDUIT2_TRACE"))
    {
        install_traces(std::make_integer_sequence<int, sizeof traced / sizeof *traced>{});
        nand_open_cb = ppc_hook("nandOpenCallback", [](PPCContext& c) {
            rt_log("nand: nandOpenCallback(result %d, block %08X)", (int32_t)c.r[3], c.r[4]);
            nand_open_cb(c);
        });
        skin_stub = ppc_hook("vSkinStub", [](PPCContext& c) {
            static uint32_t n = 0;
            if (n++ < 5 || !(n & (n - 1))) rt_log("skin: vSkinStub (call %u, r3 %08X r4 %08X r5 %08X)", n, c.r[3], c.r[4], c.r[5]);
            skin_stub(c);
        });
        skin_gqrs = ppc_hook("vSetSkinGQRs", [](PPCContext& c) {
            static uint32_t last[3];
            if (c.r[3] != last[0] || c.r[4] != last[1] || c.r[5] != last[2])
                rt_log("skin: GQR4 %08X GQR5 %08X GQR6 %08X", c.r[3], c.r[4], c.r[5]);
            last[0] = c.r[3]; last[1] = c.r[4]; last[2] = c.r[5];
            skin_gqrs(c);
        });
        reader_done = ppc_hook("cNandReader_onDone", [](PPCContext& c) {
            uint32_t r = c.r[3];
            rt_log("nand: reader %08X done(%d), status %u", r, (int32_t)c.r[4], ld32(r + 4));
            reader_done(c);
            rt_log("nand:   -> status %u, 0x178 %d", ld32(r + 4), (int32_t)ld32(r + 0x178));
        });
    }
    wpad_set_classic(true);
    wpad_set_classic_filter(mouse_on_right_stick);
}

RtGameLayer layer("Conduit 2", install);

}  // namespace
