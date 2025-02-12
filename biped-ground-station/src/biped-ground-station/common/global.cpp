#include "daemon/camera_daemon.h"
#include "daemon/inbound_daemon.h"
#include "daemon/joypad_daemon.h"
#include "daemon/logging_daemon.h"
#include "daemon/outbound_daemon.h"
#include "common/parameter.h"
#include "network/udp.h"
#include "ui/window.h"

namespace biped
{
namespace ground_station
{
std::shared_ptr<CameraDaemon> daemon_camera_ = nullptr;
std::shared_ptr<InboundDaemon> daemon_inbound_ = nullptr;
std::shared_ptr<JoypadDaemon> daemon_joypad_ = nullptr;
std::shared_ptr<LoggingDaemon> daemon_logging_ = nullptr;
std::shared_ptr<OutboundDaemon> daemon_outbound_ = nullptr;
std::string ip_biped_ = NetworkParameter::ip_biped_default;
std::shared_ptr<UDP> udp_biped_message_ = nullptr;
std::shared_ptr<UDP> udp_camera_ = nullptr;
std::shared_ptr<Window> window_ = nullptr;
}
}
