// Conduit 2 — the port's own layer over the wiikit runtime.
//
// Linked into wiiboot by conduit2.cmake. What belongs here is what only this
// game needs; later, the Classic Controller's sticks on WASD and the mouse.
#include "rt.h"
#include "video.h"
#include <cstring>

namespace {

// EXPERIMENT (session 1): a Classic Controller on channel 0. The game refuses
// a bare Remote ("A Nunchuk or Classic Controller is required"), and learns of
// controllers only from KPAD's connect callback, which the runtime accepts and
// never calls. Here the connection is announced ten frames after the game
// registers the callback (GXDrawDone, tools/conduit2-hooks.txt), and the
// Remote reports a Classic Controller. Not enough yet: the message stays, and
// the game never calls KPADReadEx or WPADProbe in the boot sequence
// (docs/08-input.md). Headed for wiikit's wpad.cpp once it works.
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
    uint32_t cl = classic_buttons(p.buttons);        // ex_status.cl
    st32(s + 0x60, cl);
    st32(s + 0x64, cl & ~prev_cl);
    st32(s + 0x68, prev_cl & ~cl);
    prev_cl = cl;
    if (err) st32(err, 0);
    c.r[3] = 1;
}

PPCFunc gx_draw_done = nullptr;
int frames_since_callback = 0;

void install() {
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
