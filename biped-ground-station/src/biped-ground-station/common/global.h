#ifndef GLOBAL_H
#define GLOBAL_H

#include <memory>
#include <mutex>

namespace biped
{
namespace ground_station
{
class CameraDaemon;
class InboundDaemon;
class JoypadDaemon;
class LoggingDaemon;
class OutboundDaemon;
class UDP;
class Window;

extern std::shared_ptr<CameraDaemon> daemon_camera_;
extern std::shared_ptr<InboundDaemon> daemon_inbound_;
extern std::shared_ptr<JoypadDaemon> daemon_joypad_;
extern std::shared_ptr<LoggingDaemon> daemon_logging_;
extern std::shared_ptr<OutboundDaemon> daemon_outbound_;
extern std::string ip_biped_;
extern std::shared_ptr<UDP> udp_biped_message_;
extern std::shared_ptr<UDP> udp_camera_;
extern std::shared_ptr<Window> window_;
}
}

#endif // GLOBAL_H
