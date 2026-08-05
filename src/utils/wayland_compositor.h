// SPDX-FileCopyrightText: 2026 CharOfString <root@charofstring.cc>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_UTILS_WAYLAND_COMPOSITOR_H_
#define SRC_UTILS_WAYLAND_COMPOSITOR_H_

#include <QByteArray>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

// Asks WM if it advertises treeland-capture-unstable-v1
inline bool compositorAdvertisesTreelandCapture() {
    if (qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
        return false;
    }

    wl_display* display = wl_display_connect(nullptr);
    if (!display) {
        return false;
    }

    bool hasCapture = false;
    wl_registry *registry = wl_display_get_registry(display);

    static const wl_registry_listener listener = {
        [](void *data, wl_registry *, uint32_t, const char *interface,
                uint32_t) {
            if (qstrcmp(interface, "treeland_capture_manager_v1") == 0) {
                *static_cast<bool *>(data) = true;
            }
        },
        [](void *, wl_registry *, uint32_t) {}
    };

    wl_registry_add_listener(registry, &listener, &hasCapture);
    wl_display_roundtrip(display);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return hasCapture;
}

#endif  // SRC_UTILS_WAYLAND_COMPOSITOR_H_
