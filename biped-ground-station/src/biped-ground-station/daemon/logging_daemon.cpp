#include <chrono>
#include <sstream>

#include "common/global.h"
#include "daemon/logging_daemon.h"
#include "common/parameter.h"
#include "utility/utility.h"
#include "ui/window.h"

namespace biped
{
namespace ground_station
{
LoggingDaemon::LoggingDaemon(QObject *parent) : QObject(parent), frame_count_(0), initialized_(false), started_(false)
{
}

LoggingDaemon::~LoggingDaemon()
{
    stop();
}

void
LoggingDaemon::start()
{
    stop();

    std::filesystem::path path_logging = std::filesystem::path(appendHomePath(expandHomePath(removeTrailingSeparator(LoggingDaemonParameter::path_logging, '/')))) / LoggingDaemonParameter::directory_name_logging;
    std::filesystem::path path_logging_data = path_logging / LoggingDaemonParameter::directory_name_logging_data;
    std::stringstream time_start_ss;
    std::time_t time_start = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    time_start_ss << std::put_time(std::localtime(&time_start), "%Y-%m-%d-%X");
    time_start_ = time_start_ss.str();
    path_logging_camera_ = path_logging / LoggingDaemonParameter::directory_name_logging_camera / time_start_;

    if (!std::filesystem::exists(path_logging_camera_))
    {
        try
        {
            std::filesystem::create_directories(path_logging_camera_);
        }
        catch (const std::filesystem::filesystem_error& error)
        {
            window_->logToStatusBar("Failed to create directories for camera frame logging.");
            return;
        }
    }

    if (!std::filesystem::exists(path_logging_data))
    {
        try
        {
            std::filesystem::create_directories(path_logging_data);
        }
        catch (const std::filesystem::filesystem_error& error)
        {
            window_->logToStatusBar("Failed to create directories for data logging.");
            return;
        }
    }

    const std::string file_name_logging_data = time_start_ + LoggingDaemonParameter::file_extension_logging_data;

    file_logging_data_.open(path_logging_data / file_name_logging_data, std::fstream::out | std::fstream::trunc);

    if (!file_logging_data_.is_open())
    {
        window_->logToStatusBar("Failed to open file for data logging.");
        return;
    }

    initialized_ = false;
    started_ = true;
}

void
LoggingDaemon::stop()
{
    started_ = false;
    initialized_ = false;

    file_logging_data_.close();
}

void
LoggingDaemon::onCameraDaemonFrameReceived(const QImage& frame)
{
    if (!started_ || !initialized_)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_frame_);
    frame_ = frame;
    frame_count_ ++;
}

void
LoggingDaemon::onInboundDaemonMessageReceived(const biped::firmware::BipedMessage& message)
{
    if (!started_ || !file_logging_data_.is_open())
    {
        return;
    }

    if (!initialized_)
    {
        file_logging_data_ << "#" << "\t"
                           << "sequence" << "\t"
                           << "timestamp" << "\t"
                           << "camera_frame" << "\t"
                           << "actuation_command.motor_enable" << "\t"
                           << "actuation_command.motor_left_forward" << "\t"
                           << "actuation_command.motor_right_forward" << "\t"
                           << "actuation_command.motor_left_pwm" << "\t"
                           << "actuation_command.motor_right_pwm" << "\t"
                           << "controller_reference.attitude_y" << "\t"
                           << "controller_reference.attitude_z" << "\t"
                           << "controller_reference.position_x" << "\t"
                           << "encoder_data.position_x" << "\t"
                           << "encoder_data.steps" << "\t"
                           << "encoder_data.steps_left" << "\t"
                           << "encoder_data.steps_right" << "\t"
                           << "encoder_data.velocity_x" << "\t"
                           << "imu_data.acceleration_x" << "\t"
                           << "imu_data.acceleration_y" << "\t"
                           << "imu_data.acceleration_z" << "\t"
                           << "imu_data.attitude_x" << "\t"
                           << "imu_data.attitude_y" << "\t"
                           << "imu_data.attitude_z" << "\t"
                           << "imu_data.angular_velocity_x" << "\t"
                           << "imu_data.angular_velocity_y" << "\t"
                           << "imu_data.angular_velocity_z" << "\t"
                           << "imu_data.compass_x" << "\t"
                           << "imu_data.compass_y" << "\t"
                           << "imu_data.compass_z" << "\t"
                           << "imu_data.temperature" << "\t"
                           << "time_of_flight_data.range_left" << "\t"
                           << "time_of_flight_data.range_middle" << "\t"
                           << "time_of_flight_data.range_right" << std::endl;

        initialized_ = true;
    }

    std::unique_lock<std::mutex> lock(mutex_frame_);
    QImage frame = frame_;
    unsigned long long frame_count = frame_count_;
    lock.unlock();

    const std::string file_name_logging_camera = std::to_string(frame_count) + LoggingDaemonParameter::file_extension_logging_camera;
    frame.save(QString::fromStdString(path_logging_camera_ / file_name_logging_camera));

    file_logging_data_ << message.sequence << "\t"
                       << message.timestamp << "\t"
                       << file_name_logging_camera << "\t"
                       << message.actuation_command.motor_enable << "\t"
                       << message.actuation_command.motor_left_forward << "\t"
                       << message.actuation_command.motor_right_forward << "\t"
                       << message.actuation_command.motor_left_pwm << "\t"
                       << message.actuation_command.motor_right_pwm << "\t"
                       << message.controller_reference.attitude_y << "\t"
                       << message.controller_reference.attitude_z << "\t"
                       << message.controller_reference.position_x << "\t"
                       << message.encoder_data.position_x << "\t"
                       << message.encoder_data.steps << "\t"
                       << message.encoder_data.steps_left << "\t"
                       << message.encoder_data.steps_right << "\t"
                       << message.encoder_data.velocity_x << "\t"
                       << message.imu_data.acceleration_x << "\t"
                       << message.imu_data.acceleration_y << "\t"
                       << message.imu_data.acceleration_z << "\t"
                       << message.imu_data.attitude_x << "\t"
                       << message.imu_data.attitude_y << "\t"
                       << message.imu_data.attitude_z << "\t"
                       << message.imu_data.angular_velocity_x << "\t"
                       << message.imu_data.angular_velocity_y << "\t"
                       << message.imu_data.angular_velocity_z << "\t"
                       << message.imu_data.compass_x << "\t"
                       << message.imu_data.compass_y << "\t"
                       << message.imu_data.compass_z << "\t"
                       << message.imu_data.temperature << "\t"
                       << message.time_of_flight_data.range_left << "\t"
                       << message.time_of_flight_data.range_middle << "\t"
                       << message.time_of_flight_data.range_right << std::endl;
}
}
}
