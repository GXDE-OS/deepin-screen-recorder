#!/bin/bash

PROTOCOL_XML="treeland-dde-shell-v1.xml"
PROTOCOL_PREFIX="treeland-dde-shell-v1"

LICENSE_HEADER="// SPDX-FileCopyrightText: 2026 GXDE Maintainers
//
// SPDX-License-Identifier: GPL-3.0-or-later
"

echo "Generating binding for ${PROTOCOL_PREFIX}..."

echo "1  // HEADERS"
wayland-scanner client-header "${PROTOCOL_XML}" "wayland-${PROTOCOL_PREFIX}-client-protocol.h"

echo "2  // IMPLS"
wayland-scanner private-code "${PROTOCOL_XML}" "wayland-${PROTOCOL_PREFIX}-protocol.c"

echo "3  // QT_HEADERS"
{
    echo "${LICENSE_HEADER}"
    /usr/lib/qt6/libexec/qtwaylandscanner client-header "${PROTOCOL_XML}" \
        | sed "s|<wayland-${PROTOCOL_PREFIX}-client-protocol.h>|\"wayland-${PROTOCOL_PREFIX}-client-protocol.h\"|"
} > "qwayland-${PROTOCOL_PREFIX}.h"

echo "4  // QT_IMPLS"
{
    echo "${LICENSE_HEADER}"
    /usr/lib/qt6/libexec/qtwaylandscanner client-code "${PROTOCOL_XML}" \
        | sed "s|<qwayland-${PROTOCOL_PREFIX}.h>|\"qwayland-${PROTOCOL_PREFIX}.h\"|"
} > "qwayland-${PROTOCOL_PREFIX}.cpp"

echo "Done."
