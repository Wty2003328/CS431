#ifndef INBOUND_DAEMON_H
#define INBOUND_DAEMON_H

#include <QObject>

#include "common/type.h"

namespace biped
{
namespace ground_station
{
class InboundDaemon : public QObject
{
    Q_OBJECT

public:

    explicit InboundDaemon(QObject *parent = nullptr);

    void
    start();

    void
    stop();

signals:

    void
    messageReceived(const biped::firmware::BipedMessage& message);

public slots:

    void
    operate();

private:

    bool started_;
};
}
}

#endif // INBOUND_DAEMON_H
