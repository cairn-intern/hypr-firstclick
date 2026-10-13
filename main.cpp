// Copyright © 2026 euxaristia
// SPDX-License-Identifier: Apache-2.0

#define WLR_USE_UNSTABLE

#include <any>
#include <set>

#include <linux/input-event-codes.h>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/devices/IKeyboard.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

// macOS-style click-to-focus: the first click on an unfocused window only
// focuses it. The click is not delivered to the window, so it cannot press
// buttons, follow links, or place a cursor by accident. The matching release
// is swallowed too, so the app never sees a release without a press.

inline HANDLE PHANDLE = nullptr;

static SP<HOOK_CALLBACK_FN> g_mouseHook;
static std::set<uint32_t>   g_swallowedButtons;

static bool enabled() {
    static auto* const* PENABLED = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:firstclick:enabled")->getDataStaticPtr();
    return **PENABLED != 0;
}

static bool anyModifierDown() {
    for (const auto& kb : g_pInputManager->m_keyboards) {
        if (kb && kb->getModifiers() != 0)
            return true;
    }
    return false;
}

static void onMouseButton([[maybe_unused]] void* self, SCallbackInfo& info, std::any data) {
    if (!enabled())
        return;

    IPointer::SButtonEvent event;
    try {
        event = std::any_cast<IPointer::SButtonEvent>(data);
    } catch (...) {
        return;
    }

    if (event.button != BTN_LEFT && event.button != BTN_RIGHT && event.button != BTN_MIDDLE)
        return;

    if (event.state == WL_POINTER_BUTTON_STATE_RELEASED) {
        if (g_swallowedButtons.erase(event.button) > 0)
            info.cancelled = true;
        return;
    }

    // Leave modified clicks alone: Super+drag window moves, Ctrl+click, and
    // mouse keybinds must keep working on the first press.
    if (anyModifierDown())
        return;

    const auto TARGET = g_pCompositor->vectorToWindowUnified(g_pInputManager->getMouseCoordsInternal(),
                                                              Desktop::View::RESERVED_EXTENTS | Desktop::View::INPUT_EXTENTS | Desktop::View::ALLOW_FLOATING);
    if (!TARGET)
        return;

    if (TARGET == Desktop::focusState()->window())
        return;

    g_swallowedButtons.insert(event.button);
    Desktop::focusState()->fullWindowFocus(TARGET);
    // Cancelling the button skips Hyprland's normal pointer refocus, which
    // leaves the cursor without a surface and it disappears. Resync it.
    g_pInputManager->refocus();
    info.cancelled = true;
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH        = __hyprland_api_get_hash();
    const std::string CLIENT_HASH = __hyprland_api_get_client_hash();

    if (HASH != CLIENT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[firstclick] Version mismatch (headers do not match running Hyprland)", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[firstclick] Version mismatch");
    }

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:firstclick:enabled", Hyprlang::INT{1});

    g_mouseHook = HyprlandAPI::registerCallbackDynamic(PHANDLE, "mouseButton", ::onMouseButton);
    if (!g_mouseHook) {
        HyprlandAPI::addNotification(PHANDLE, "[firstclick] Failed to register mouseButton hook", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[firstclick] Failed to register mouseButton hook");
    }

    HyprlandAPI::reloadConfig();

    return {"firstclick", "First click on an unfocused window only focuses it", "euxaristia", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_mouseHook.reset();
    g_swallowedButtons.clear();
}
