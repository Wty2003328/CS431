#ifndef UDP_H
#define UDP_H

#include <boost/asio.hpp>
#include <memory>
#include <mutex>

namespace biped
{
namespace ground_station
{
class UDP
{
public:

    UDP(const std::string& ip_local, const uint16_t& port);

    UDP(const uint16_t& port);

    ~UDP();

    bool
    bound();

    void
    close();

    void
    open(boost::asio::ip::udp::endpoint endpoint_local);

    std::string
    read(const std::string& ip_remote, const uint16_t& port, const size_t& size);

    std::shared_ptr<std::vector<char>>
    readBuffer(const std::string& ip_remote, const uint16_t& port, const size_t& size);

    size_t
    write(const std::string& ip_remote, const uint16_t& port, const std::string& data);

    size_t
    writeBuffer(const std::string& ip_remote, const uint16_t& port, const uint8_t* buffer, const size_t& size);

private:

    bool bound_;
    std::mutex mutex_read_;
    std::mutex mutex_write_;
    boost::asio::thread_pool thread_pool_;
    boost::asio::ip::udp::socket socket_;
};
}
}

#endif // UDP_H
