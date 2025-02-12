#include "common/global.h"
#include "daemon/inbound_daemon.h"
#include "common/parameter.h"
#include "network/udp.h"
#include "ui/window.h"

namespace biped
{
namespace ground_station
{
InboundDaemon::InboundDaemon(QObject *parent) : QObject(parent), started_(false)
{
}

void
InboundDaemon::start()
{
    started_ = true;
}

void
InboundDaemon::stop()
{
    started_ = false;
}

void
InboundDaemon::operate()
{
    for (;;)
    {
        if (!started_)
        {
            break;
        }

        if (!udp_biped_message_ || !udp_biped_message_->bound())
        {
            continue;
        }

        std::string message = udp_biped_message_->read(ip_biped_, NetworkParameter::port_udp_biped_message, NetworkParameter::buffer_size_biped_message);

        if (message == "")
        {
            continue;
        }

        biped::firmware::BipedMessage message_deserialized;
        std::vector<unsigned char> message_serialized(message.begin(), message.end());
        zpp::serializer::memory_input_archive deserializer(message_serialized);

        const auto result = deserializer(message_deserialized);

        if (!result)
        {
            if (window_)
            {
                window_->logToStatusBar("Failed to deserialize Biped message.");
            }

            continue;
        }

        message_deserialized.controller_parameter.pid_controller_saturation_position_x.input_lower = message_deserialized.controller_parameter.pid_controller_saturation_position_x.input_lower < -UIParameter::window_rendering_value_limit ? -UIParameter::window_rendering_value_limit : message_deserialized.controller_parameter.pid_controller_saturation_position_x.input_lower;
        message_deserialized.controller_parameter.pid_controller_saturation_position_x.input_upper = message_deserialized.controller_parameter.pid_controller_saturation_position_x.input_upper > UIParameter::window_rendering_value_limit ? UIParameter::window_rendering_value_limit : message_deserialized.controller_parameter.pid_controller_saturation_position_x.input_upper;

        emit messageReceived(message_deserialized);
    }
}
}
}
