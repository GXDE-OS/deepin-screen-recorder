/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     He Mingyang<hemingyang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "scrollScreenshot.h"

#include <QDebug>
#include <QDateTime>
#include <X11/Xlibint.h>
#include <X11/extensions/XTest.h>

ScrollScreenshot::ScrollScreenshot(QObject *parent)  : QObject(parent)
{
    Q_UNUSED(parent);
    qRegisterMetaType<PixMergeThread::MergeErrorValue>("MergeErrorValue");

    m_mouseWheelTimer = new QTimer(this);
    connect(m_mouseWheelTimer, &QTimer::timeout, this, [ = ] {
        // 发送滚轮事件， 自动滚动
        static Display *m_display = XOpenDisplay(nullptr);
        XTestFakeButtonEvent(m_display, 5, 1, CurrentTime);
        XFlush(m_display);
        XTestFakeButtonEvent(m_display, 5, 0, CurrentTime);
        XFlush(m_display);
        //当模拟鼠标进行自动滚动时，会发射此信号
        emit autoScroll(m_autoScrollFlag++);
        // 滚动区域高度 200-300 取值2
        // 滚动区域高度 > 300  取值 3
        // 滚动区域高度 > 600  取值 5
        m_scrollCount++;
        if (m_scrollCount % m_shotFrequency == 0)
        {
            emit getOneImg();
        }
    });

    m_PixMerageThread = new PixMergeThread(this);
    connect(m_PixMerageThread, SIGNAL(updatePreviewImg(QImage)), this, SIGNAL(updatePreviewImg(QImage)));
    connect(m_PixMerageThread, SIGNAL(merageError(PixMergeThread::MergeErrorValue)), this, SLOT(merageImgState(PixMergeThread::MergeErrorValue)));
    connect(m_PixMerageThread, &PixMergeThread::invalidAreaError, this, &ScrollScreenshot::merageInvalidArea);
}


void ScrollScreenshot::addPixmap(const QPixmap &piximg, int wheelDirection)
{
    if (m_startPixMerageThread == false) {
        m_PixMerageThread->start();
        m_startPixMerageThread = true;
    }
    if (m_isManualScrollModel == false) {//自动
        if (m_curStatus == Wait) {
            m_PixMerageThread->setScrollModel(false);
            m_mouseWheelTimer->start(300);
            m_curStatus = Merging;
        }
        if (m_curStatus == Merging) {
            m_PixMerageThread->addShotImg(piximg, PixMergeThread::PictureDirection::ScrollDown);
        }
    } else if (m_isManualScrollModel == true) {//手动
        //if (piximg.isNull() == true)
        //qDebug() << "function piximg is null: " << __func__ << " ,line: " << __LINE__;
        //if (m_curStatus == Wait) {
        m_PixMerageThread->setScrollModel(true);
        m_mouseWheelTimer->stop();
        // m_curStatus = Merging;
        // }
        //if (m_curStatus == Merging) {
        PixMergeThread::PictureDirection  status = (wheelDirection == WheelDown) ? (PixMergeThread::PictureDirection::ScrollDown) : (PixMergeThread::PictureDirection::ScrollUp);
        m_PixMerageThread->addShotImg(piximg, status);
        // }
    }
}

void ScrollScreenshot::clearPixmap()
{
    m_PixMerageThread->clearCurImg();
}

void ScrollScreenshot::changeState(const bool isStop)
{
    //qDebug() << __FUNCTION__ << "====" << isStop;
    // 暂停
    if (isStop && m_curStatus == Merging) {
        m_curStatus = Stop;
        m_mouseWheelTimer->stop();
    }

    // 开始
    if (!isStop && m_curStatus == Stop) {
        m_curStatus = Merging;
        m_mouseWheelTimer->start(300);
    }
}
QImage ScrollScreenshot::savePixmap()
{
    if (m_curStatus == ScrollStatus::Merging) {
        m_mouseWheelTimer->stop();
        emit getOneImg();
        m_PixMerageThread->stopTask();
        m_PixMerageThread->wait();
    }
    //QDateTime currentDate;
    //QString currentTime =  currentDate.currentDateTime().toString("yyyyMMddHHmmss");
    //QString saveFileName = QString("%1_%2_%3.%4").arg("S", "Long IMG", currentTime, "png");
    //(m_PicMerageThread->getMerageResult()).save(saveFileName);
    return m_PixMerageThread->getMerageResult();
}


//设置滚动模式，先设置滚动模式，再添加图片
void ScrollScreenshot::setScrollModel(bool model)
{
    m_isManualScrollModel = model;
}
//获取调整区域
QRect ScrollScreenshot::getInvalidArea()
{
    return m_rect;
}
//设置时间并计算时间差
void ScrollScreenshot::setTimeAndCalculateTimeDiff(int time)
{
    m_PixMerageThread->calculateTimeDiff(time);
}


void ScrollScreenshot::merageImgState(PixMergeThread::MergeErrorValue state)
{
    qDebug() << "拼接状态值:" << state;
    m_mouseWheelTimer->stop();
    if (state == PixMergeThread::MaxHeight) {
        m_curStatus = ScrollStatus::Mistake;
    }
    emit merageError(state);
}

//调整捕捉区域
void ScrollScreenshot::merageInvalidArea(PixMergeThread::MergeErrorValue state, QRect rect)
{
    m_rect = rect;
    m_mouseWheelTimer->stop();
    if (state == PixMergeThread::MaxHeight) {
        m_curStatus = ScrollStatus::Mistake;
    }
    emit merageError(state);
}
