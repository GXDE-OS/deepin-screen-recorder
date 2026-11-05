// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TABLERECOGNIZERLOADER_H
#define TABLERECOGNIZERLOADER_H

#include <QImage>
#include <QObject>
#include <QString>

// 表格识别能力由 dtkmultimedia 的 dtk6tablerecognizer 提供，走常规链接。
// 该库并非所有架构都有（例如 i386 / riscv64），pin_screenshots.pro 检测到它时才链接
// 并定义 TABLE_RECOGNIZER_ON；否则本类退化为“不可用”，界面上也不提供表格识别入口。
// 头文件里不直接引用库的类型，调用方无需关心库是否存在。
#ifdef TABLE_RECOGNIZER_ON
namespace Dtk {
namespace TableRecognizer {
class DTableRecognizer;
}
}
#endif

/**
 * @brief 表格识别能力（dtkmultimedia）的封装
 *
 * 识别结束后把结果和错误码交给展示层，展示层只依据错误码选择提示文案，
 * 不对库的 errorMessage 做任何字符串匹配。
 */
class TableRecognizerLoader : public QObject
{
    Q_OBJECT
public:
    // 与 Dtk::TableRecognizer::TableError 一一对应，另加库缺失时的 Unavailable。
    enum class Error {
        None,
        InvalidImage,
        NoTableDetected,
        NoTextDetected,
        Timeout,
        Busy,
        OcrEngineUnavailable,
        OcrFailed,
        InternalError,
        Unavailable, // 构建时没有 dtk6tablerecognizer
    };
    Q_ENUM(Error)

    explicit TableRecognizerLoader(QObject *parent = nullptr);

    // 当前构建是否带有表格识别能力
    static bool isAvailable();

    /**
     * @brief 异步识别表格
     * @param image 待识别图片
     * @param timeoutMs 识别超时时间（毫秒）
     * @return 是否成功启动识别流程
     */
    bool recognize(const QImage &image, int timeoutMs = 25000);

Q_SIGNALS:
    void recognitionFinished(bool success, const QString &html, TableRecognizerLoader::Error error);

private:
#ifdef TABLE_RECOGNIZER_ON
    Dtk::TableRecognizer::DTableRecognizer *m_recognizer = nullptr;
#endif
};

#endif // TABLERECOGNIZERLOADER_H
