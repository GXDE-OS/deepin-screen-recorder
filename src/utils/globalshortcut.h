// SPDX-FileCopyrightText: 2026 CharOfString <root@charofstring.cc>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_UTILS_GLOBALSHORTCUT_H_
#define SRC_UTILS_GLOBALSHORTCUT_H_

#include <QObject>
#include <QString>
#include <QStringList>

class GlobalShortcut : public QObject {
    Q_OBJECT

public:
    GlobalShortcut(const QString& componentName,
        const QString& componentFriendlyName, QObject* parent = nullptr);
    static bool isSupported();
    bool bind(const QString& actionName, const QString& actionFriendlyName,
        QKeyCombination keys);

Q_SIGNALS:
    void triggered(const QString& actionName);

private Q_SLOTS:
    void onGlobalShortcutPressed(const QString& componentUnique,
        const QString& actionUnique, qlonglong timestamp);

private:
    QStringList actionId(const QString& actionName,
        const QString& actionFriendlyName) const;
    bool connectComponent();

    QString m_componentName;
    QString m_componentFriendlyName;
    bool m_connected = false;
};

#endif  // SRC_UTILS_GLOBALSHORTCUT_H_
