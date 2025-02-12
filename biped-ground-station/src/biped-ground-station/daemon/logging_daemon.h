#ifndef LOGGING_DAEMON_H
#define LOGGING_DAEMON_H

#include <fstream>
#include <mutex>
#include <QImage>
#include <QObject>
#include <filesystem> // filesystem must be included after Qt includes due to a bug in MOC: https://bugreports.qt.io/browse/QTBUG-73263

#include "common/type.h"

namespace biped
{
namespace ground_station
{
class LoggingDaemon : public QObject
{
    Q_OBJECT

public:

    explicit LoggingDaemon(QObject *parent = nullptr);

    ~LoggingDaemon();

    void
    start();

    void
    stop();

public slots:

    void
    onCameraDaemonFrameReceived(const QImage& frame);

    void
    onInboundDaemonMessageReceived(const biped::firmware::BipedMessage& message);

private:

    std::fstream file_logging_data_;
    QImage frame_;
    unsigned long long frame_count_;
    bool initialized_;
    std::mutex mutex_frame_;
    std::filesystem::path path_logging_camera_;
    bool started_;
    std::string time_start_;
};
}
}

#endif // LOGGING_DAEMON_H
