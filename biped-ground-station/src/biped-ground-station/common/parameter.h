#ifndef PARAMETER_H
#define PARAMETER_H

#include <cstddef>
#include <cstdint>

namespace biped
{
namespace ground_station
{
namespace JoypadDaemonParameter
{
constexpr double deadzone_factor = 0.01;
constexpr char path_device_default[] = "/dev/input/js0";
constexpr size_t timeout = 200000;
constexpr size_t value_limit = 32767;
}

namespace LoggingDaemonParameter
{
constexpr char path_logging[] = "~/Downloads";
constexpr char directory_name_logging[] = "biped";
constexpr char directory_name_logging_camera[] = "camera";
constexpr char directory_name_logging_data[] = "data";
constexpr char file_extension_logging_camera[] = ".jpg";
constexpr char file_extension_logging_data[] = ".log";
}

namespace NetworkParameter
{
constexpr size_t buffer_size_biped_message = 1024;
constexpr size_t buffer_size_camera = 1460;
constexpr char camera_frame_boundary[] = "123456789000000000000987654321";
constexpr char ip_biped_default[] = "192.168.0.100";
constexpr uint16_t port_udp_biped_message = 4431;
constexpr uint16_t port_udp_camera = 4432;
constexpr size_t timeout = 200;
}

namespace UIParameter
{
constexpr double confirm_dialog_text_resize_factor = 1.1;
constexpr double confirm_dialog_text_informative_resize_factor = 0.8;
constexpr size_t confirm_dialog_button_width_minimum = 70;
constexpr size_t parameter_dialog_text_edit_height_fixed = 90;
constexpr char parameter_set_push_button_pin_style_pinned[] = "background-color: rgb(223, 106, 106); color: rgb(246, 246, 227);";
constexpr unsigned rolling_plot_rolling_window_capacity_default = 50;
constexpr unsigned window_parameter_label_current_style_reset_delay = 5000;
constexpr char window_parameter_label_current_style_updated[] = "color: rgb(0, 200, 0);";
constexpr unsigned window_rendering_fps_cap_default_biped_message = 60;
constexpr unsigned window_rendering_fps_cap_default_camera_frame = 60;
constexpr double window_rendering_value_limit = 9999;
}
}
}

#endif // PARAMETER_H
