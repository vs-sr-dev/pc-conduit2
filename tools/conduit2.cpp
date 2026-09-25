// Conduit 2 — the port's own layer over the wiikit runtime.
//
// Linked into wiiboot by conduit2.cmake. What belongs here is what only this
// game needs; later, the Classic Controller's sticks on WASD and the mouse.
#include "rt.h"
#include "video.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <cstring>

namespace {

// A Classic Controller on channel 0. The game learns of controllers only from
// KPAD's connect callback, which the runtime accepts and never calls: the
// connection is announced ten frames after the game registers the callback
// (GXDrawDone, tools/conduit2-hooks.txt), and the Remote reports a Classic
// Controller. ("A Nunchuk or Classic Controller is required" is a fixed
// notice of the boot, shown in Dolphin too.) Played with in session 2: menus,
// WASD, mouse look. Headed for wiikit's wpad.cpp (docs/08-input.md).
constexpr uint32_t WPAD_DEV_CLASSIC = 2, WPAD_FMT_CLASSIC_BTN_ACC_DPD = 8;

void stf(uint32_t a, float f) { uint32_t u; std::memcpy(&u, &f, 4); st32(a, u); }

uint32_t classic_buttons(uint32_t core) {
    static const uint32_t map[][2] = {{0x0800, 0x0010}, {0x0400, 0x0040}, {0x0008, 0x0001}, {0x0004, 0x4000},
                                      {0x0001, 0x0002}, {0x0002, 0x8000}, {0x0010, 0x0400}, {0x1000, 0x1000},
                                      {0x0200, 0x0008}, {0x0100, 0x0020}};   // A B Up Down Left Right + - 1:X 2:Y
    uint32_t cl = 0;
    for (auto& m : map)
        if (core & m[0]) cl |= m[1];
    return cl;
}

// EXPERIMENT: the Classic Controller straight from SDL's keyboard and mouse
// (the key file knows only the Remote's buttons). The layout is a guess until
// the game's controls screen is read: WASD the left stick, the mouse's motion
// the right stick, the mouse buttons ZR and ZL.
enum : uint32_t { CL_UP = 0x0001, CL_LEFT = 0x0002, CL_ZR = 0x0004, CL_X = 0x0008, CL_A = 0x0010,
                  CL_Y = 0x0020, CL_B = 0x0040, CL_ZL = 0x0080, CL_R = 0x0200, CL_PLUS = 0x0400,
                  CL_HOME = 0x0800, CL_MINUS = 0x1000, CL_L = 0x2000, CL_DOWN = 0x4000, CL_RIGHT = 0x8000 };
struct ClassicKeys { uint32_t buttons; float lx, ly, rx, ry, lt, rt; };
float g_mouse_gain = 0.15f;                          // stick deflection per pixel of motion per sample (CONDUIT2_MOUSE)

ClassicKeys classic_from_host() {
    ClassicKeys k{};
    if (!SDL_GetKeyboardFocus()) return k;
    const bool* ks = SDL_GetKeyboardState(nullptr);
    static const struct { SDL_Scancode sc; uint32_t bit; } keys[] = {
        {SDL_SCANCODE_RETURN, CL_A}, {SDL_SCANCODE_SPACE, CL_A}, {SDL_SCANCODE_BACKSPACE, CL_B},
        {SDL_SCANCODE_C, CL_B}, {SDL_SCANCODE_R, CL_X}, {SDL_SCANCODE_F, CL_Y}, {SDL_SCANCODE_E, CL_R},
        {SDL_SCANCODE_LSHIFT, CL_L}, {SDL_SCANCODE_Q, CL_MINUS}, {SDL_SCANCODE_TAB, CL_PLUS},
        {SDL_SCANCODE_H, CL_HOME}, {SDL_SCANCODE_UP, CL_UP}, {SDL_SCANCODE_DOWN, CL_DOWN},
        {SDL_SCANCODE_LEFT, CL_LEFT}, {SDL_SCANCODE_RIGHT, CL_RIGHT}};
    for (auto& e : keys)
        if (ks[e.sc]) k.buttons |= e.bit;
    k.lx = (ks[SDL_SCANCODE_D] ? 1.0f : 0.0f) - (ks[SDL_SCANCODE_A] ? 1.0f : 0.0f);
    k.ly = (ks[SDL_SCANCODE_W] ? 1.0f : 0.0f) - (ks[SDL_SCANCODE_S] ? 1.0f : 0.0f);
    if (k.lx && k.ly) { k.lx *= 0.7071f; k.ly *= 0.7071f; }
    float dx = 0, dy = 0;
    video_take_mouse_motion(dx, dy);
    SDL_MouseButtonFlags mb = SDL_GetMouseState(nullptr, nullptr);
    k.rx = std::clamp(dx * g_mouse_gain, -1.0f, 1.0f);
    k.ry = std::clamp(-dy * g_mouse_gain, -1.0f, 1.0f);
    if (mb & SDL_BUTTON_LMASK) k.buttons |= CL_ZR;
    if (mb & SDL_BUTTON_RMASK) k.buttons |= CL_ZL;
    if (k.buttons & CL_L) k.lt = 1.0f;
    if (k.buttons & CL_R) k.rt = 1.0f;
    return k;
}

uint32_t prev_core = 0, prev_cl = 0;
uint32_t connect_cb = 0, extension_cb = 0;
bool announced = false;

// the game marks a channel connected from KPAD's connect callback, and learns
// the extension from WPAD's extension callback: call both, once, as a Remote
// with a Classic Controller attached would
void announce(const PPCContext& c) {
    announced = true;
    if (connect_cb) {
        PPCContext e = c;
        e.r[3] = 0;                                  // chan
        e.r[4] = 0;                                  // WPAD_ERR_NONE: connected
        ppc_call_indirect(e, connect_cb);
        rt_log("conduit2: channel 0 connected (callback %08X)", connect_cb);
    }
    if (extension_cb) {
        PPCContext e = c;
        e.r[3] = 0;
        e.r[4] = WPAD_DEV_CLASSIC;
        ppc_call_indirect(e, extension_cb);
        rt_log("conduit2: Classic Controller attached (callback %08X)", extension_cb);
    }
}

void kpad_read_ex(PPCContext& c) {                   // (chan, KPADStatus*, len, s32* err)
    uint32_t chan = c.r[3], s = c.r[4], len = c.r[5], err = c.r[6];
    if (chan != 0 || !s || !len) {
        if (err) st32(err, (uint32_t)-1);
        c.r[3] = 0;
        return;
    }
    if (!announced && connect_cb) announce(c);
    PadState p = video_pad();
    for (uint32_t i = 0; i < 0xF0; i += 4) st32(s + i, 0);
    st32(s + 0x00, p.buttons);
    st32(s + 0x04, p.buttons & ~prev_core);
    st32(s + 0x08, prev_core & ~p.buttons);
    prev_core = p.buttons;
    stf(s + 0x10, -1.0f);                            // acc: level, gravity along -y
    stf(s + 0x18, 1.0f);
    stf(s + 0x20, p.x);
    stf(s + 0x24, p.y);
    stf(s + 0x34, 1.0f);                             // horizon
    stf(s + 0x48, 2.0f);                             // dist
    st8(s + 0x5C, WPAD_DEV_CLASSIC);
    st8(s + 0x5E, p.pointer ? 2 : 0);
    st8(s + 0x5F, WPAD_FMT_CLASSIC_BTN_ACC_DPD);
    ClassicKeys k = classic_from_host();             // ex_status.cl
    uint32_t cl = k.buttons;
    st32(s + 0x60, cl);
    st32(s + 0x64, cl & ~prev_cl);
    st32(s + 0x68, prev_cl & ~cl);
    prev_cl = cl;
    stf(s + 0x6C, k.lx);                              // lstick
    stf(s + 0x70, k.ly);
    stf(s + 0x74, k.rx);                              // rstick
    stf(s + 0x78, k.ry);
    stf(s + 0x7C, k.lt);                              // ltrigger, rtrigger
    stf(s + 0x80, k.rt);
    if (err) st32(err, 0);
    c.r[3] = 1;
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

PPCFunc gx_draw_done = nullptr;
int frames_since_callback = 0;

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
    gx_draw_done = ppc_hook("GXDrawDone", [](PPCContext& c) {
        gx_draw_done(c);
        if (!announced && connect_cb && ++frames_since_callback == 10) announce(c);
    });
    // the SDK is Victorious's own builds (Aug 23 2010): KPADStatus is the
    // runtime's default 0xF0
    ppc_hook("WPADProbe", [](PPCContext& c) {
        if (!announced && connect_cb) announce(c);
        bool here = c.r[3] == 0;
        if (c.r[4]) st32(c.r[4], here ? WPAD_DEV_CLASSIC : 253);
        c.r[3] = here ? 0 : (uint32_t)-1;
    });
    ppc_hook("KPADReadEx", kpad_read_ex);
    ppc_hook("KPADSetConnectCallback", [](PPCContext& c) {
        uint32_t old = connect_cb;
        if (c.r[3] == 0) connect_cb = c.r[4];
        c.r[3] = old;
    });
    ppc_hook("WPADSetExtensionCallback", [](PPCContext& c) {
        uint32_t old = extension_cb;
        if (c.r[3] == 0) extension_cb = c.r[4];
        c.r[3] = old;
    });
}

RtGameLayer layer("Conduit 2", install);

}  // namespace
