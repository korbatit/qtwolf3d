#include <QtGui>

#include "wl_def.h"
#include "wlplay.h"
#include "main.h"

#include "display.h"

extern WLPlay* wlplay;

extern QSize   g_screenSize;

WLDisplay::WLDisplay(QWidget *parent, WLInput* input)
    : QWidget(parent), m_input(input)
{
    setAttribute(Qt::WA_StaticContents);
    setBackgroundRole(QPalette::Text);
    setAutoFillBackground(true);
    resize(320,200);
    grabKeyboard();
    m_image = new QImage(640, 400, QImage::Format_RGB32);
    g_screenSize = QSize(640, 400);
    update();
}

void WLDisplay::keyPressEvent(QKeyEvent *event)
{
    //if (event->key() == Qt::Key_S)
    //    wlplay->screenShot();

    m_input->keyDown(event);
}

void WLDisplay::keyReleaseEvent(QKeyEvent *event)
{
    m_input->keyUp(event);
}

void WLDisplay::mousePressEvent(QMouseEvent *event)
{
    m_input->mouseDown(event);
}

void WLDisplay::mouseMoveEvent(QMouseEvent *event)
{
    m_input->mouseMove(event);
}

void WLDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    m_input->mouseUp(event);
}

void WLDisplay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    if (m_image) {
#ifdef TOUCHSCREEN
        painter.setBackground(Qt::black);
        painter.drawImage( (g_screenSize.width()-m_image->width())/2,
                           (g_screenSize.height()-m_image->height())/2,
                           *m_image);
#else
        painter.drawImage(rect(), *m_image, m_image->rect(), Qt::ThresholdDither);
#endif
    }
}

void WLDisplay::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    g_screenSize = size();
}

void WLDisplay::setImage(QImage *img)
{
    m_image = img;
    repaint();
}
