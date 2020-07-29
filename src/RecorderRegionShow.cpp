#include <QBitmap>
#include <QVector>
#include "RecorderRegionShow.h"
#include "utils.h"

const int INDICATOR_WIDTH = 110;
const int CAMERA_Y_OFFSET = 40;

RecorderRegionShow::RecorderRegionShow():m_cameraWidget(nullptr)
{
    setAttribute (Qt::WA_AlwaysShowToolTips);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_painter =  new QPainter();
}
RecorderRegionShow::~RecorderRegionShow()
{
}

void RecorderRegionShow::initCameraInfo(const CameraWidget::Position position, const QSize size)
{
    m_cameraWidget = new CameraWidget();
    QRect r = this->geometry();
    m_cameraWidget->setFixedSize(size);
    m_cameraWidget->initCamera();
    m_cameraWidget->setRecordRect(r.x(), r.y(), r.width(), r.height());
    switch (position) {
        case CameraWidget::Position::rightBottom:{
            m_cameraWidget->showAt(QPoint(r.x() + r.width() - m_cameraWidget->width(), r.y() + r.height() - m_cameraWidget->height() + CAMERA_Y_OFFSET));
            break;
        }
        case CameraWidget::Position::rightTop:{
            m_cameraWidget->showAt(QPoint(r.x() + r.width() - m_cameraWidget->width(), r.y() + CAMERA_Y_OFFSET));
            break;
        }
        case CameraWidget::Position::leftBottom:{
            m_cameraWidget->showAt(QPoint(r.x(), r.y() + r.height() - m_cameraWidget->height() + CAMERA_Y_OFFSET));
            break;
        }
        case CameraWidget::Position::leftTop:{
            m_cameraWidget->showAt(QPoint(r.x(), r.y() + CAMERA_Y_OFFSET));
            break;
        }
    }

    m_cameraWidget->cameraStart();
    Utils::passInputEvent(static_cast<int>(m_cameraWidget->winId()));
}

void RecorderRegionShow::showKeyBoardButtons(const QString &key)
{
    KeyButtonWidget *t_keyWidget = new KeyButtonWidget();
    t_keyWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
    t_keyWidget->setKeyLabelWord(key);
    m_keyButtonList.append(t_keyWidget);

    if (m_keyButtonList.count() > 5) {
        delete m_keyButtonList.first();
        m_keyButtonList.pop_front();
    }
    //更新多按钮的位置
    updateMultiKeyBoardPos();
    repaint();
}

void RecorderRegionShow::paintEvent(QPaintEvent *event)
{
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::transparent);
    m_painter->begin( &pixmap );
    m_painter->setRenderHints( QPainter::Antialiasing, true);

    QPen pen(Qt::white, 2.0);
    pen.setStyle(Qt::DashLine);
    QVector<qreal> dashes;
    dashes << 1 << 2;
    pen.setDashPattern(dashes);
    pen.setDashOffset(0);

    m_painter->setPen(pen);
    m_painter->setOpacity(1);
    m_painter->drawRect(0, 0, width(), height());
    m_painter->end();

    m_painter->begin(this);
    m_painter->drawPixmap(QPoint(0, 0), pixmap);
    m_painter->end();
    setMask(pixmap.mask());
    event->accept();
}
void RecorderRegionShow::updateMultiKeyBoardPos()
{
    QPoint t_keyPoint1;
    QPoint t_keyPoint2;
    QPoint t_keyPoint3;
    QPoint t_keyPoint4;
    QPoint t_keyPoint5;

    QRect r = this->geometry();
    int recordX = r.x();
    int recordY = r.y();
    int recordWidth = r.width();
    int recordHeight = r.height();

    if (!m_keyButtonList.isEmpty()) {
        switch (m_keyButtonList.count()) {
        //一个按键的情况
        case 1:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();
            break;
            //两个按键的情况
        case 2:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 + m_keyButtonList.at(1)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();
            break;
            //三个按键的情况o
        case 3:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();
            break;
            //四个按键的情况
        case 4:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2 + m_keyButtonList.at(2)->width() / 1.5),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();
            break;
            //五个按键的情况
        case 5:
            m_keyButtonList.at(0)->hide();
            t_keyPoint1 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(0)->width() / 2 - m_keyButtonList.at(0)->width() * 2.6),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(0)->move(t_keyPoint1.x(), t_keyPoint1.y());
            m_keyButtonList.at(0)->show();

            m_keyButtonList.at(1)->hide();
            t_keyPoint2 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(1)->width() / 2 - m_keyButtonList.at(1)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(1)->move(t_keyPoint2.x(), t_keyPoint2.y());
            m_keyButtonList.at(1)->show();

            m_keyButtonList.at(2)->hide();
            t_keyPoint3 = QPoint(recordX + recordWidth / 2 - m_keyButtonList.at(2)->width() / 2,
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(2)->move(t_keyPoint3.x(), t_keyPoint3.y());
            m_keyButtonList.at(2)->show();

            m_keyButtonList.at(3)->hide();
            t_keyPoint4 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(3)->width() * 1.3),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(3)->move(t_keyPoint4.x(), t_keyPoint4.y());
            m_keyButtonList.at(3)->show();

            m_keyButtonList.at(4)->hide();
            t_keyPoint5 = QPoint(static_cast<int>(recordX + recordWidth / 2 - m_keyButtonList.at(3)->width() / 2 + m_keyButtonList.at(4)->width() * 2.6),
                                 std::max(recordY + recordHeight - INDICATOR_WIDTH, 0));
            m_keyButtonList.at(4)->move(t_keyPoint5.x(), t_keyPoint5.y());
            m_keyButtonList.at(4)->show();
            break;
        default:
            break;
        }
    }
}
