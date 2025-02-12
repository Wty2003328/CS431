#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>

#include "daemon/joypad_daemon.h"
#include "common/parameter.h"

namespace biped
{
namespace ground_station
{
JoypadDaemon::JoypadDaemon(QObject *parent) : QObject(parent), device_(-1), initialized_(false), started_(false)
{
    openDevice(JoypadDaemonParameter::path_device_default);
}

JoypadDaemon::~JoypadDaemon()
{
    closeDevice();
}

void
JoypadDaemon::start()
{
    started_ = true;
}

void
JoypadDaemon::stop()
{
    started_ = false;
}

void
JoypadDaemon::operate()
{
    for (;;)
    {
        if (!started_)
        {
            break;
        }

        if (!initialized_)
        {
            openDevice(JoypadDaemonParameter::path_device_default);
            continue;
        }

        ssize_t bytes;
        js_event event;
        fd_set set;
        int status;
        timeval timeout;

        FD_ZERO(&set);
        FD_SET(device_, &set);

        timeout.tv_sec = 0;
        timeout.tv_usec = JoypadDaemonParameter::timeout;

        status = select(device_ + 1, &set, NULL, NULL, &timeout);

        if (status <= 0)
        {
            continue;
        }

        bytes = read(device_, &event, sizeof(event));

        if (bytes != sizeof(event))
        {
            continue;
        }

        switch (event.type)
        {
        case JS_EVENT_AXIS:
        {
            if (event.number / 2 == 0)
            {
                if (event.number % 2 == 0)
                {
                    double axis_x = event.value;

                    if (std::fabs(axis_x) < JoypadDaemonParameter::deadzone_factor * JoypadDaemonParameter::value_limit)
                    {
                        axis_x = 0;
                    }

                    axis_x /= JoypadDaemonParameter::value_limit;
                    axis_x = std::min<double>(std::max<double>(axis_x, -1), 1);

                    emit xChanged(axis_x);
                }
                else
                {
                    double axis_y = event.value;

                    if (std::fabs(axis_y) < JoypadDaemonParameter::deadzone_factor * JoypadDaemonParameter::value_limit)
                    {
                        axis_y = 0;
                    }

                    axis_y *= -1;
                    axis_y /= JoypadDaemonParameter::value_limit;
                    axis_y = std::min<double>(std::max<double>(axis_y, -1), 1);

                    emit yChanged(axis_y);
                }
            }

            break;
        }
        default:
        {
            break;
        }
        }
}
}

bool
JoypadDaemon::closeDevice()
{
    initialized_ = false;

    if (device_ >= 0)
    {
        if (close(device_) < 0)
        {
            return false;
        }
    }

    return true;
}

bool
JoypadDaemon::openDevice(const std::string& path)
{
    closeDevice();

    device_ = open(path.c_str(), O_RDONLY);

    if (device_ < 0)
    {
        return initialized_;
    }

    initialized_ = true;

    return initialized_;
}
}
}
