// SPDX-FileCopyrightText: 2026 CharOfString <root@charofstring.cc>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalshortcut.h"
#include "log.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QList>

namespace {

const char* kService = "org.kde.kglobalaccel";
const char* kRegistryPath = "/kglobalaccel";
const char* kRegistryInterface = "org.kde.KGlobalAccel";
const char* kComponentInterface = "org.kde.kglobalaccel.Component";

// KGlobalAccel::SetShortcutFlag
enum SetShortcutFlag {
    SetPresent = 2,
    NoAutoloading = 4,
    IsDefault = 8,
};

// KGlobalAccel encodes a QKeySequence as a fixed four int struct (ai). Without
// KF6GlobalAccel that marshalling has to be done by hand.
struct DBusKeySequence {
    QList<int> keys;
};

QDBusArgument &operator<<(QDBusArgument& argument,
        const DBusKeySequence& sequence) {
    argument.beginStructure();
    argument << sequence.keys;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument& argument,
        DBusKeySequence& sequence) {
    argument.beginStructure();
    argument >> sequence.keys;
    argument.endStructure();
    return argument;
}

}  // namespace

Q_DECLARE_METATYPE(DBusKeySequence)
Q_DECLARE_METATYPE(QList<DBusKeySequence>)

GlobalShortcut::GlobalShortcut(const QString& componentName,
        const QString& componentFriendlyName, QObject* parent)
            : QObject(parent)
            , m_componentName(componentName)
            , m_componentFriendlyName(componentFriendlyName) {
    qDBusRegisterMetaType<DBusKeySequence>();
    qDBusRegisterMetaType<QList<DBusKeySequence>>();
}

bool GlobalShortcut::isSupported() {
    QDBusConnectionInterface* bus = QDBusConnection::sessionBus().interface();
    if (!bus) {
        return false;
    }

    return bus->isServiceRegistered(QLatin1String(kService));
}

QStringList GlobalShortcut::actionId(
        const QString& actionName, const QString& actionFriendlyName) const {
    return { m_componentName, actionName, m_componentFriendlyName,
        actionFriendlyName };
}

bool GlobalShortcut::bind(const QString& actionName,
        const QString& actionFriendlyName, QKeyCombination keys) {
    const QStringList id = actionId(actionName, actionFriendlyName);
    const int keyCode = keys.toCombined();

    QDBusInterface registry(QLatin1String(kService),
        QLatin1String(kRegistryPath),
        QLatin1String(kRegistryInterface),
        QDBusConnection::sessionBus());

    if (!registry.isValid()) {
        return false;
    }

    QDBusReply<void> registered =
        registry.call(QStringLiteral("doRegister"), id);

    if (!registered.isValid()) {
        return false;
    }

    DBusKeySequence sequence;
    sequence.keys = { keyCode, 0, 0, 0 };

    QDBusReply<QList<DBusKeySequence>> result = registry.call(
        QStringLiteral("setShortcutKeys"), id,
        QVariant::fromValue(QList<DBusKeySequence>{ sequence }),
        static_cast<uint>(SetPresent | NoAutoloading));

    if (!result.isValid()) {
        return false;
    }

    const QList<DBusKeySequence> assigned = result.value();
    if (assigned.isEmpty() || assigned.first().keys.value(0) != keyCode) {
        return false;
    }

    if (!connectComponent()) {
        return false;
    }

    return true;
}

bool GlobalShortcut::connectComponent() {
    if (m_connected) {
        return true;
    }

    QDBusInterface registry(QLatin1String(kService),
        QLatin1String(kRegistryPath),
        QLatin1String(kRegistryInterface),
        QDBusConnection::sessionBus());

    QDBusReply<QDBusObjectPath> component =
        registry.call(QStringLiteral("getComponent"), m_componentName);

    if (!component.isValid()) {
        qCWarning(dsrApp) << "getComponent failed:"
            << component.error().message();
        return false;
    }

    m_connected = QDBusConnection::sessionBus().connect(
        QLatin1String(kService), component.value().path(),
        QLatin1String(kComponentInterface),
        QStringLiteral("globalShortcutPressed"), this,
        SLOT(onGlobalShortcutPressed(QString, QString, qlonglong)));

    if (!m_connected) {
        return false;
    }

    return true;
}

void GlobalShortcut::onGlobalShortcutPressed(const QString& componentUnique,
        const QString& actionUnique, qlonglong timestamp) {
    Q_UNUSED(timestamp)
    if (componentUnique != m_componentName) {
        return;
    }

    Q_EMIT triggered(actionUnique);
}
