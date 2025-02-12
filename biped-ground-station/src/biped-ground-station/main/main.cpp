#include <iostream>
#include <QApplication>

#include "daemon/camera_daemon.h"
#include "common/global.h"
#include "daemon/inbound_daemon.h"
#include "daemon/joypad_daemon.h"
#include "daemon/logging_daemon.h"
#include "daemon/outbound_daemon.h"
#include "common/parameter.h"
#include "common/type.h"
#include "network/udp.h"
#include "ui/window.h"

using namespace biped::ground_station;

int
main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    daemon_camera_ = std::make_shared<CameraDaemon>();
    daemon_inbound_ = std::make_shared<InboundDaemon>();
    daemon_joypad_ = std::make_shared<JoypadDaemon>();
    daemon_logging_ = std::make_shared<LoggingDaemon>();
    daemon_outbound_ = std::make_shared<OutboundDaemon>();
    udp_biped_message_ = std::make_shared<UDP>(NetworkParameter::port_udp_biped_message);
    udp_camera_ = std::make_shared<UDP>(NetworkParameter::port_udp_camera);
    window_ = std::make_shared<Window>();

    const int status = application.exec();

    window_.reset();

    return status;
}
