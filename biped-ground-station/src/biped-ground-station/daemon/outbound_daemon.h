#ifndef OUTBOUND_DAEMON_H
#define OUTBOUND_DAEMON_H

#include <QObject>

#include "common/type.h"

namespace biped
{
namespace ground_station
{
class OutboundDaemon : public QObject
{
    Q_OBJECT

public:

    explicit OutboundDaemon(QObject *parent = nullptr);

public slots:

    void
    operate(const biped::firmware::BipedMessage& message);
};
}
}

#endif // OUTBOUND_DAEMON_H
