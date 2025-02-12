#include "common/parameter.h"
#include "network/udp.h"

namespace biped
{
namespace ground_station
{
UDP::UDP(const std::string& ip_local, const uint16_t& port) : bound_(true), socket_(thread_pool_)
{
    open(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(ip_local), port));
}

UDP::UDP(const uint16_t& port) : bound_(true), socket_(thread_pool_)
{
    open(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
}

UDP::~UDP()
{
    close();
}

bool
UDP::bound()
{
    return bound_;
}

void
UDP::close()
{
    bound_ = false;

    thread_pool_.join();
    socket_.close();
}

void
UDP::open(boost::asio::ip::udp::endpoint endpoint_local)
{
    socket_.open(boost::asio::ip::udp::v4());

    try
    {
        socket_.bind(endpoint_local);
    }
    catch (const std::exception exception)
    {
        bound_ = false;
    }
}

std::string
UDP::read(const std::string& ip_remote, const uint16_t& port, const size_t& size)
{
    std::shared_ptr<std::vector<char>> buffer = readBuffer(ip_remote, port, size);

    if (buffer)
    {
        return std::string(buffer->begin(), buffer->end());
    }
    else
    {
        return "";
    }
}

std::shared_ptr<std::vector<char>>
UDP::readBuffer(const std::string& ip_remote, const uint16_t& port, const size_t& size)
{
    if (!bound())
    {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex_read_);

    std::shared_ptr<std::vector<char>> buffer = std::make_shared<std::vector<char>>(size, '0');
    boost::asio::ip::udp::endpoint endpoint_remote;

    std::future future = socket_.async_receive_from(boost::asio::buffer(*buffer), endpoint_remote, boost::asio::use_future);

    switch (future.wait_for(std::chrono::milliseconds(NetworkParameter::timeout)))
    {
    case std::future_status::ready:
    {
        break;
    }
    default:
    {
        socket_.cancel();
        return nullptr;
    }
    }

    if (endpoint_remote.address().to_string() != ip_remote || endpoint_remote.port() != port)
    {
        return nullptr;
    }

    return buffer;
}

size_t
UDP::write(const std::string& ip_remote, const uint16_t& port, const std::string& data)
{
    return writeBuffer(ip_remote, port, reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

size_t
UDP::writeBuffer(const std::string& ip_remote, const uint16_t& port, const uint8_t* buffer, const size_t& size)
{
    size_t bytes = 0;

    if (!bound())
    {
        return bytes;
    }

    std::lock_guard<std::mutex> lock(mutex_write_);

    boost::asio::ip::udp::endpoint endpoint_remote(boost::asio::ip::address::from_string(ip_remote), port);

    bytes = socket_.send_to(boost::asio::buffer(buffer, size), endpoint_remote);

    return bytes;
}
}
}
