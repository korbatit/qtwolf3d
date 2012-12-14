#ifndef WLDISPLAY_H
#define WLDISPLAY_H

#include <QImage>
#include <QWidget>

#include "wlinput.h"

class WLDisplay : public QWidget
{
    Q_OBJECT
public:
    WLDisplay(QWidget *parent = 0, WLInput* input = 0);
    ~WLDisplay() {}

    QImage* image() const { return m_image; }
    void setImage(QImage* img);

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QImage*  m_image;
    WLInput* m_input;
};

#endif
