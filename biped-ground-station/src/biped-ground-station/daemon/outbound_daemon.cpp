#include "common/global.h"
#include "daemon/outbound_daemon.h"
#include "common/parameter.h"
#include "network/udp.h"
#include "ui/window.h"

namespace biped
{
namespace ground_station
{
OutboundDaemon::OutboundDaemon(QObject *parent) : QObject(parent)
{
}

void
OutboundDaemon::operate(const biped::firmware::BipedMessage& message)
{
    if (!udp_biped_message_ || !udp_biped_message_->bound())
    {
        return;
    }

    std::vector<unsigned char> message_serialized;
    zpp::serializer::memory_output_archive serializer(message_serialized);

    const auto result = serializer(message);

    if (result)
    {
        udp_biped_message_->write(ip_biped_, NetworkParameter::port_udp_biped_message, std::string(message_serialized.begin(), message_serialized.end()));

        if (window_)
        {
            window_->logToStatusBar("Sent Biped message to Biped at \"" + ip_biped_ + "\".");
        }
    }
    else
    {
        if (window_)
        {
            window_->logToStatusBar("Failed to serialize Biped message.");
        }
    }
}
}
}
