#ifndef JOYPAD_DAEMON_H
#define JOYPAD_DAEMON_H

#include <QObject>

namespace biped
{
namespace ground_station
{
class JoypadDaemon : public QObject
{
    Q_OBJECT

public:

    explicit JoypadDaemon(QObject *parent = nullptr);

    ~JoypadDaemon();

    void
    start();

    void
    stop();

signals:

    void
    xChanged(const double& value);

    void
    yChanged(const double& value);

public slots:

    void
    operate();

private:

    bool
    closeDevice();

    bool
    openDevice(const std::string& path);

    int device_;
    bool initialized_;
    bool started_;
};
}
}

#endif // JOYPAD_DAEMON_H
