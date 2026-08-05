// SPDX-FileCopyrightText: 2026 CharOfString <root@charofstring.cc>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddeshellsurface.h"
#include "../utils/log.h"

#include "qwayland-treeland-dde-shell-v1.h"

#include <private/qwaylandclientextension_p.h>
#include <private/qwaylandwindow_p.h>

#include <QGuiApplication>
#include <QHash>
#include <QWindow>

namespace {

class ShellManager : public QWaylandClientExtensionTemplate<ShellManager>
        , public QtWayland::treeland_dde_shell_manager_v1 {
public:
    ShellManager() : QWaylandClientExtensionTemplate<ShellManager>(1) {}
    using QWaylandClientExtension::initialize;
};

ShellManager *shellManager() {
    static ShellManager manager;
    return &manager;
}

class OverlaySurface : public QtWayland::treeland_dde_shell_surface_v1 {
public:
    explicit OverlaySurface(struct ::treeland_dde_shell_surface_v1* object)
            : QtWayland::treeland_dde_shell_surface_v1(object) {}

    ~OverlaySurface() override {
        if (isInitialized()) {
            destroy();
        }
    }
};

struct OverlayEntry {
    OverlaySurface* shellSurface = nullptr;
    struct ::wl_surface* surface = nullptr;
};

QHash<QWindow *, OverlayEntry> g_overlays;

void releaseOverlay(QWindow* window) {
    const OverlayEntry entry = g_overlays.take(window);
    delete entry.shellSurface;
}

}  // namespace

bool DDEShell::raiseToOverlay(QWindow* window) {
    if (!window) {
        return false;
    }

    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"),
            Qt::CaseInsensitive)) {
        return false;
    }

    auto* platformWindow = static_cast<QtWaylandClient::QWaylandWindow *>(
        window->handle());
    if (!platformWindow) {
        return false;
    }

    struct ::wl_surface* surface = platformWindow->surface();
    if (!surface) {
        return false;
    }

    const OverlayEntry existing = g_overlays.value(window);
    if (existing.shellSurface) {
        if (existing.surface == surface) {
            return true;
        }
        releaseOverlay(window);
    }

    ShellManager* manager = shellManager();
    if (!manager->isActive()) {
        manager->initialize();
    }

    if (!manager->isActive()) {
        return false;
    }

    auto* shellSurface = new OverlaySurface(
        manager->get_shell_surface(surface));
    shellSurface->set_role(
        QtWayland::treeland_dde_shell_surface_v1::role_overlay);
    g_overlays.insert(window, { shellSurface, surface });

    if (existing.shellSurface == nullptr) {
        QObject::connect(window, &QObject::destroyed, window, [window] {
            releaseOverlay(window);
        });
    }

    return true;
}
