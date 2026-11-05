// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tablerecognizerloader.h"

#include <QDebug>

#include <chrono>

#include "../../utils/log.h"

#ifdef TABLE_RECOGNIZER_ON
#include <dtablerecognizer.h>

namespace {
TableRecognizerLoader::Error toLoaderError(Dtk::TableRecognizer::TableError error)
{
    using E = Dtk::TableRecognizer::TableError;
    switch (error) {
    case E::None: return TableRecognizerLoader::Error::None;
    case E::InvalidImage: return TableRecognizerLoader::Error::InvalidImage;
    case E::NoTableDetected: return TableRecognizerLoader::Error::NoTableDetected;
    case E::NoTextDetected: return TableRecognizerLoader::Error::NoTextDetected;
    case E::Timeout: return TableRecognizerLoader::Error::Timeout;
    case E::Busy: return TableRecognizerLoader::Error::Busy;
    case E::OcrEngineUnavailable: return TableRecognizerLoader::Error::OcrEngineUnavailable;
    case E::OcrFailed: return TableRecognizerLoader::Error::OcrFailed;
    case E::InternalError: return TableRecognizerLoader::Error::InternalError;
    }
    return TableRecognizerLoader::Error::InternalError;
}
} // namespace
#endif

TableRecognizerLoader::TableRecognizerLoader(QObject *parent)
    : QObject(parent)
#ifdef TABLE_RECOGNIZER_ON
    , m_recognizer(new Dtk::TableRecognizer::DTableRecognizer(this))
#endif
{
#ifdef TABLE_RECOGNIZER_ON
    connect(m_recognizer, &Dtk::TableRecognizer::DTableRecognizer::recognitionDone, this,
            [this](const Dtk::TableRecognizer::DTableResult &result) {
                qCDebug(dsrApp) << "[TABLE] recognition done. success:" << result.success
                                << "source:" << result.source
                                << "error:" << static_cast<int>(result.error)
                                << "message:" << result.errorMessage
                                << "html size:" << result.html.size();
                emit recognitionFinished(result.success, result.html, toLoaderError(result.error));
            });
#endif
}

bool TableRecognizerLoader::isAvailable()
{
#ifdef TABLE_RECOGNIZER_ON
    return true;
#else
    return false;
#endif
}

bool TableRecognizerLoader::recognize(const QImage &image, int timeoutMs)
{
#ifdef TABLE_RECOGNIZER_ON
    if (image.isNull()) {
        emit recognitionFinished(false, QString(), Error::InvalidImage);
        return false;
    }
    qCDebug(dsrApp) << "[TABLE] recognition started, timeout(ms):" << timeoutMs;
    m_recognizer->recognizeAsync(image, std::chrono::milliseconds(timeoutMs));
    return true;
#else
    Q_UNUSED(image)
    Q_UNUSED(timeoutMs)
    qCWarning(dsrApp) << "[TABLE] built without dtk6tablerecognizer, table recognition unavailable";
    emit recognitionFinished(false, QString(), Error::Unavailable);
    return false;
#endif
}
