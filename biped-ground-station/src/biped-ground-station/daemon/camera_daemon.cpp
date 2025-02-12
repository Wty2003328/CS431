#include "common/global.h"
#include "daemon/camera_daemon.h"
#include "common/parameter.h"
#include "network/udp.h"

namespace biped
{
namespace ground_station
{
CameraDaemon::CameraDaemon(QObject *parent) : QObject(parent), started_(false)
{
}

void
CameraDaemon::start()
{
    started_ = true;
}

void
CameraDaemon::stop()
{
    started_ = false;
}

void
CameraDaemon::operate()
{
    bool frame_boundary_received = false;
    const std::string frame_boundary_reference = NetworkParameter::camera_frame_boundary;
    bool frame_incomplete = false;
    size_t frame_packet_count = 0;

    for (;;)
    {
        if (!started_)
        {
            break;
        }

        if (!udp_camera_ || !udp_camera_->bound())
        {
            continue;
        }

        std::shared_ptr<std::vector<char>> buffer = udp_camera_->readBuffer(ip_biped_, NetworkParameter::port_udp_camera, NetworkParameter::buffer_size_camera);

        if (buffer)
        {
            if (!frame_boundary_received)
            {
                if (buffer->size() <= frame_boundary_reference.size())
                {
                    continue;
                }

                const std::string frame_boundary = std::string(buffer->begin(), buffer->begin() + frame_boundary_reference.size());

                if (frame_boundary == frame_boundary_reference)
                {
                    frame_boundary_received = true;
                    frame_packet_count = static_cast<size_t>(buffer->at(frame_boundary_reference.size()));
                }
            }

            if (frame_boundary_received)
            {
                frame_boundary_received = false;

                std::vector<char> jpg_buffer;

                for (size_t i = 0; i < frame_packet_count; i ++)
                {
                    buffer = udp_camera_->readBuffer(ip_biped_, NetworkParameter::port_udp_camera, NetworkParameter::buffer_size_camera);

                    if (!buffer)
                    {
                        frame_incomplete = true;
                        break;
                    }

                    if (buffer->size() > frame_boundary_reference.size())
                    {
                        const std::string frame_boundary = std::string(buffer->begin(), buffer->begin() + frame_boundary_reference.size());

                        if (frame_boundary == frame_boundary_reference)
                        {
                            frame_incomplete = true;
                            break;
                        }
                    }

                    jpg_buffer.insert(jpg_buffer.end(), buffer->begin(), buffer->end());
                }

                frame_packet_count = 0;

                if (frame_incomplete)
                {
                    frame_incomplete = false;
                    continue;
                }

                if (frame_.loadFromData(reinterpret_cast<unsigned char*>(jpg_buffer.data()), jpg_buffer.size(), "JPG"))
                {
                    emit frameReceived(frame_);
                }
            }
        }
    }
}
}
}
