#ifndef CAMERA_VIEW_H
#define CAMERA_VIEW_H

#include <QOpenGLWidget>

namespace biped
{
namespace ground_station
{
class CameraView : public QOpenGLWidget
{
    Q_OBJECT

public:

    explicit CameraView(QWidget* parent = nullptr);

public slots:

    void
    onCameraFrame(const QImage& frame);

protected:

    void
    paintEvent(QPaintEvent *event) override;

private:

    QImage frame_;
};
}
}

#endif // CAMERA_VIEW_H
