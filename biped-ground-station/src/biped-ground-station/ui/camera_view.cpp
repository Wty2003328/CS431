#include <QPaintEvent>
#include <QPainter>

#include "ui/camera_view.h"

namespace biped
{
namespace ground_station
{
CameraView::CameraView(QWidget* parent) : QOpenGLWidget(parent)
{
}

void
CameraView::onCameraFrame(const QImage& frame)
{
    frame_ = frame;
    update();
}

void
CameraView::paintEvent(QPaintEvent *event)
{
    QPainter painter;

    painter.begin(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::LosslessImageRendering);
    painter.drawImage(event->rect(), frame_);

    painter.end();
}
}
}
