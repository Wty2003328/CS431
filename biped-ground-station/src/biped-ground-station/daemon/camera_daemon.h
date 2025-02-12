#ifndef CAMERA_DAEMON_H
#define CAMERA_DAEMON_H

#include <QImage>
#include <QObject>

namespace biped
{
namespace ground_station
{
class CameraDaemon : public QObject
{
    Q_OBJECT

public:

    explicit CameraDaemon(QObject *parent = nullptr);

    void
    start();

    void
    stop();

signals:

    void
    frameReceived(const QImage& frame);

public slots:

    void
    operate();

private:

    QImage frame_;
    bool started_;
};
}
}

#endif // CAMERA_DAEMON_H
