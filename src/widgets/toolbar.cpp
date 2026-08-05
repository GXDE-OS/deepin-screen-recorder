// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbar.h"
#include "../utils/baseutils.h"
#include "../utils/saveutils.h"
#include "../utils.h"
#include "../utils/configsettings.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"
#include "../utils/log.h"

#include <DIconButton>

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>
#include <QBitmap>

#include <dgraphicsgloweffect.h>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 68;
const int TOOLBAR_WIDTH = 425;
}

ToolBarWidget::ToolBarWidget(MainWindow *pMainwindow, DWidget *parent, bool hideToolBar)
    : DFloatingWidget(parent)
    , m_hideToolBar(hideToolBar)
{
    qCDebug(dsrApp) << "ToolBarWidget constructor called.";
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    blurBackground()->setBlendMode(DBlurEffectWidget::InWidgetBlend);

    if (Utils::themeType == 1) {
        blurBackground()->setMaskColor(QColor(0xF7, 0xF7, 0xF7, 153));
    } else {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }

    m_hSeparatorLine = new DLabel(this);
    m_hSeparatorLine->setObjectName("HorSeparatorLine");
    m_hSeparatorLine->setFixedHeight(1);
    //分配pMainwindow主窗口指针给SubToolWidget（ToolTips需要该指针）
    m_subTool = new SubToolWidget(pMainwindow, this);
    // m_subTool->setVisible(false);
    //关闭按钮
    m_closeButton = new ToolButton(this);
    m_closeButton->setCheckable(false);
    m_confirmButton = new ToolButton(this);
    m_confirmButton->setCheckable(false);

    m_closeButton->setIconSize(QSize(36, 36));
    m_confirmButton->setIconSize(QSize(36, 36));
    m_closeButton->setFixedSize(QSize(36, 36));
    m_confirmButton->setFixedSize(QSize(36, 36));
    m_closeButton->setIcon(QIcon::fromTheme("close"));
    m_confirmButton->setIcon(QIcon::fromTheme("confirm"));

    m_subTool->installTipHint(m_closeButton, tr("Close (Esc)"));
    m_subTool->installTipHint(m_confirmButton, tr("Copy to clipboard (Enter)"));

    Utils::setAccessibility(m_closeButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL);
    Utils::setAccessibility(m_confirmButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL);

    // 当从uos ai的调用过来则将该窗口隐藏
    if (false)
        m_subTool->setVisible(false);

    setFixedHeight(TOOLBAR_HEIGHT);
    if (Utils::is3rdInterfaceStart) {
        qCDebug(dsrApp) << "Setting minimum width for 3rd interface mode";
        m_subTool->setMinimumWidth(TOOLBAR_WIDTH - 160); //减去隐藏按钮的最小宽度和
    }
    qWarning()<<"hideToolBar"<<m_hideToolBar;
    if (m_hideToolBar)
    {
        m_subTool->hide();
    }
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    hLayout->addSpacing(6);
    hLayout->addWidget(m_closeButton, 0,  Qt::AlignCenter);
    hLayout->addWidget(m_confirmButton, 0,  Qt::AlignCenter);
    hLayout->addSpacing(10);
    setLayout(hLayout);

    connect(m_confirmButton, &ToolButton::clicked, pMainwindow, &MainWindow::confirm);

    connect(m_closeButton, &ToolButton::clicked, pMainwindow, [=](){
        if (m_hideToolBar) {
            emit pMainwindow->screenshotSaved("");
        }
        pMainwindow->exitApp();
    });
    connect(m_subTool, &SubToolWidget::keyBoardButtonClicked, pMainwindow, &MainWindow::changeKeyBoardShowEvent);
    connect(m_subTool, &SubToolWidget::mouseBoardButtonClicked, pMainwindow, &MainWindow::changeMouseShowEvent);

    // TODO:treeland适配
    connect(m_subTool, &SubToolWidget::shotOptionMenuShown, pMainwindow, &MainWindow::handleOptionMenuShown);

    QMetaObject::Connection connectHandle =  connect(m_subTool, SIGNAL(cameraActionChecked(bool)), pMainwindow, SLOT(changeCameraSelectEvent(bool)));
    if (!connectHandle) {
        qCDebug(dsrApp) << "Connect cameraActionChecked signal failed!";
    }
    connect(m_subTool, &SubToolWidget::changeShotToolFunc, pMainwindow, &MainWindow::changeShotToolEvent);
    connect(m_subTool, &SubToolWidget::changeShotToolFunc, this, &ToolBarWidget::changeShotToolEvent);
}

void ToolBarWidget::setScrollShotDisabled(const bool state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setScrollShotDisabled called with state:" << state;
    m_subTool->setScrollShotDisabled(state);
}

void ToolBarWidget::setPinScreenshotsEnable(const bool &state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setPinScreenshotsEnable called with state:" << state;
    m_subTool->setPinScreenshotsEnable(state);

}

void ToolBarWidget::setOcrScreenshotsEnable(const bool &state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setOcrScreenshotsEnable called with state:" << state;
    m_subTool->setOcrScreenshotEnable(state);
}

void ToolBarWidget::setButEnableOnLockScreen(const bool &state)
{
    qCDebug(dsrApp) << "ToolBarWidget::setButEnableOnLockScreen called with state:" << state;
    m_subTool->setButEnableOnLockScreen(state);
}

int ToolBarWidget::getFuncSubToolX(QString &func)
{
    qCDebug(dsrApp) << "ToolBarWidget::getFuncSubToolX called for function:" << func;
    return m_subTool->getFuncSubToolX(func);
}


QRect ToolBarWidget::getAiButtonGlobalRect() const
{
    return m_subTool->getAiButtonGlobalRect();
}

QPoint ToolBarWidget::getAiButtonGlobalCenter() const
{
    return m_subTool->getAiButtonGlobalCenter();
}

void ToolBarWidget::setHideToolbar(bool hidetoolbar)
{
    m_hideToolBar = hidetoolbar;
}

void ToolBarWidget::setRecordLaunchFromMain(const unsigned int funType)
{
    qCDebug(dsrApp) << "ToolBarWidget::setRecordLaunchFromMain called with function type:" << funType;
    m_subTool->setRecordLaunchMode(funType);

}

void ToolBarWidget::setRecordButtonDisable()
{
    qCDebug(dsrApp) << "ToolBarWidget::setRecordButtonDisable called.";
   // m_subTool->setRecordButtonDisable();
}

void ToolBarWidget::setVideoInitFromMain()
{
    qCDebug(dsrApp) << "ToolBarWidget::setVideoInitFromMain called.";
    m_subTool->setVideoButtonInitFromSub();
}

void ToolBarWidget::shapeClickedFromBar(QString shape)
{
    qCDebug(dsrApp) << "ToolBarWidget::shapeClickedFromBar called with shape:" << shape;
    m_subTool->shapeClickedFromWidget(shape);
}

void ToolBarWidget::setCameraDeviceEnable(bool status)
{
    qCDebug(dsrApp) << "ToolBarWidget::setCameraDeviceEnable called with status:" << status;
    m_subTool->setCameraDeviceEnable(status);
}

void ToolBarWidget::changeShotToolEvent(const QString &func)
{
    if(func.isEmpty() || !m_confirmButton || !m_subTool)
        return;

    qCWarning(dsrApp) << "===============func============================="<< func;
    if (func == "record")
        m_subTool->installTipHint(m_confirmButton, tr("OK (Enter)"));
    else if (func == "shot")
        m_subTool->installTipHint(m_confirmButton, tr("Copy to clipboard (Enter)"));
}
/*
void ToolBarWidget::setExpand(bool expand, QString shapeType)
{
    Q_UNUSED(expand);
//    m_subToolbar->switchContent(shapeType);
    m_subTool->switchContent(shapeType);
//    emit expandChanged(expand, shapeType);

//    if (expand) {
//        m_expanded = true;
//        setFixedSize(TOOLBAR_WIDGET_SIZE.width(),
//                                 TOOLBAR_WIDGET_SIZE.height()*2+1);
//        m_hSeparatorLine->show();
//        m_subToolbar->show();
//    }
    emit changeFunctionSignal(shapeType);
    update();
}
*/

QRect ToolBarWidget::getShotOptionRect(){
    qCDebug(dsrApp) << "ToolBarWidget::getShotOptionRect called.";
   return m_subTool->getShotOptionRect();
}

ToolBarWidget::~ToolBarWidget() {
    qCDebug(dsrApp) << "ToolBarWidget destructor called.";
}


ToolBar::ToolBar(DWidget *parent)
    : DLabel(parent)
{
    //setAttribute(Qt::WA_TranslucentBackground);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    m_toolbarWidget = nullptr;
}

void ToolBar::paintEvent(QPaintEvent *e)
{
    DLabel::paintEvent(e);
}

QRect ToolBar::getInnerWidgetRect() const
{
    if (!m_toolbarWidget)
        return rect();
    // 返回 ToolBarWidget 的 contentsRect 在 ToolBar 坐标系下的位置
    QRect cr = m_toolbarWidget->contentsRect();
    return QRect(m_toolbarWidget->mapTo(const_cast<ToolBar *>(this), cr.topLeft()), cr.size());
}

#if (QT_VERSION_MAJOR == 5)
void ToolBar::enterEvent(QEvent *e)
{
    //    qApp->setOverrideCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
    DLabel::enterEvent(e);
}
#elif (QT_VERSION_MAJOR == 6)
void ToolBar::enterEvent(QEnterEvent *e)
{
    //    qApp->setOverrideCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::OpenHandCursor);
    DLabel::enterEvent(e);
}
#endif

bool ToolBar::eventFilter(QObject *obj, QEvent *event)
{
    return DLabel::eventFilter(obj, event);
}

void ToolBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        m_isPress = true;
        //记下鼠标在工具栏内的抓取偏移（不能用 globalPos，理由见 mouseMoveEvent）
        m_mouseStartPoint = event->pos();
        //获得窗口的初始位置
        m_windowStartPoint = this->frameGeometry().topLeft();
        m_hasDragAnchor = false;
        m_pMainWindow->getSideBarStartPressPoint();
    }

}

void ToolBar::mouseMoveEvent(QMouseEvent *event)
{
    //判断是否在拖拽移动
    if (m_isPress) {
        m_isDrag = true;
        // wayland 下事件坐标有两个来源：工具栏自己的 surface，以及主窗口那个全屏 overlay
        // （工具栏的 windowHandle 在 main_window.cpp updateCaptureRegion 里被挂成了它的子
        // surface，指针拖出工具栏后事件从主窗口送达，Qt 转发时不补 subsurface 偏移）。
        // 前者要靠 pos() 才能换算成屏幕坐标，而 pos() 是客户端自己记的、拖拽中会和合成器
        // 实际摆放的位置脱节，换算出来的位置能差出上百像素，两簇交替就是肉眼可见的抖动。
        // 所以只认后者：主窗口是全屏的，原点即屏幕原点，这簇坐标本身就是指针的屏幕位置。
        const QPoint pointerPos = event->pos();
        if (!m_hasDragAnchor) {
            // 第一帧要挑一个主窗口坐标系的事件做锚点。工具栏自身那簇必然落在自己矩形
            // 附近，按矩形外扩一圈排除（不能用 rect().contains()：它的边界是 height-1，
            // 指针停在下边缘那一行时 y 恰好等于 height，会漏过去）。
            if (rect().adjusted(-width(), -height(), width(), height()).contains(pointerPos)) {
                return;
            }
            m_dragAnchor = pointerPos;
            m_lastPointer = pointerPos;
            m_hasDragAnchor = true;
            return;
        }
        // 锚定之后靠连续性分簇：两簇坐标相差上百像素，正常拖拽相邻帧不会跳这么远。
        // 注意跳变要当成「参考系换了」处理，就地重新锚定后继续跟；早先的版本是直接丢弃，
        // 结果事件流整体切簇时会被连续拒绝几百帧（实测最长冻结 462ms），手感发呆。
        if ((pointerPos - m_lastPointer).manhattanLength() > 300) {
            m_dragAnchor = pointerPos - (m_lastPointer - m_dragAnchor);
            m_lastPointer = pointerPos;
            return;
        }
        m_lastPointer = pointerPos;
        const QPoint move_distance = pointerPos - m_dragAnchor;
        if (m_pMainWindow) {
            m_pMainWindow->moveToolBars(m_windowStartPoint, move_distance);
        }
    }
}

void ToolBar::mouseReleaseEvent(QMouseEvent *event)
{
    //放下左键即停止移动
    if (event->button() == Qt::LeftButton) {
        m_isPress = false;
        m_isDrag = false;
        QApplication::setOverrideCursor(Qt::OpenHandCursor);
    }
}

void ToolBar::setScrollShotDisabled(const bool state)
{
    if (!m_toolbarWidget) {
        return;
    }
    m_toolbarWidget->setScrollShotDisabled(state);
}

void ToolBar::setPinScreenshotsEnable(const bool &state)
{
    if (m_toolbarWidget)
        m_toolbarWidget->setPinScreenshotsEnable(state);
}

void ToolBar::setOcrScreenshotsEnable(const bool &state)
{
    if (m_toolbarWidget)
        m_toolbarWidget->setOcrScreenshotsEnable(state);
}

void ToolBar::setButEnableOnLockScreen(const bool &state)
{
    if (m_toolbarWidget)
        m_toolbarWidget->setButEnableOnLockScreen(state);
}

int ToolBar::getFuncSubToolX(QString &func)
{
    return m_toolbarWidget ? m_toolbarWidget->getFuncSubToolX(func) : 0;
}


QRect ToolBar::getAiButtonGlobalRect() const
{
    return m_toolbarWidget ? m_toolbarWidget->getAiButtonGlobalRect() : QRect();
}

QPoint ToolBar::getAiButtonGlobalCenter() const
{
    return m_toolbarWidget ? m_toolbarWidget->getAiButtonGlobalCenter() : QPoint();
}

bool ToolBar::isDraged()
{
    return m_isDrag;
}

bool ToolBar::isPressed()
{
    return m_isPress;
}

void ToolBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}


void ToolBar::currentFunctionMode(QString shapeType)
{       
    qCDebug(dsrApp) << "Setting current function mode:" << shapeType;
    DPalette pa;
    update();
    emit currentFunctionToMain(shapeType);
}

void ToolBar::initToolBar(MainWindow *pmainWindow, bool hideToolbar)
{
    m_pMainWindow = pmainWindow;
    setFixedHeight(TOOLBAR_HEIGHT);

    m_toolbarWidget = new ToolBarWidget(pmainWindow, this, hideToolbar);
    QHBoxLayout *vLayout = new QHBoxLayout();
    vLayout->setSizeConstraint(QLayout::SetFixedSize);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addStretch();
    vLayout->addWidget(m_toolbarWidget);
    vLayout->addStretch();
    setLayout(vLayout);
    update();

    connect(m_toolbarWidget, &ToolBarWidget::changeFunctionSignal, this, &ToolBar::currentFunctionMode);
}

void ToolBar::showWidget(){
    if(m_toolbarWidget)
        m_toolbarWidget->show();
}

void ToolBar::hideWidget(){
    if(m_toolbarWidget)
        m_toolbarWidget->hide();
}

void ToolBar::setRecordButtonDisable()
{
    if (m_toolbarWidget)
        m_toolbarWidget->setRecordButtonDisable();
}

void ToolBar::setRecordLaunchMode(const unsigned int funType)
{
    qCDebug(dsrApp) << "Setting record launch mode:" << funType;
    if (m_toolbarWidget)
        m_toolbarWidget->setRecordLaunchFromMain(funType);
}

void ToolBar::setVideoButtonInit()
{
    if (m_toolbarWidget)
        m_toolbarWidget->setVideoInitFromMain();
}

void ToolBar::shapeClickedFromMain(QString shape)
{
    // 在工具栏显示之前，触发MainWindow::initShortcut()中的快捷键
    // R O L P T
    // 导致button没有初始化就执行click，导致崩溃。
    if (m_toolbarWidget) {
        m_toolbarWidget->shapeClickedFromBar(shape);
    }
}


void ToolBar::setCameraDeviceEnable(bool status)
{
    qCDebug(dsrApp) << "Setting camera device enable:" << status;
    if (m_toolbarWidget)
        m_toolbarWidget->setCameraDeviceEnable(status);
}

QRect ToolBar::getShotOptionRect(){
   return m_toolbarWidget ? m_toolbarWidget->getShotOptionRect() : QRect();
}
ToolBar::~ToolBar()
{
}

