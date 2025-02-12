#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QThread>

#include "common/global.h"
#include "daemon/camera_daemon.h"
#include "ui/confirm_dialog.h"
#include "daemon/inbound_daemon.h"
#include "daemon/joypad_daemon.h"
#include "daemon/logging_daemon.h"
#include "daemon/outbound_daemon.h"
#include "common/parameter.h"
#include "ui/parameter_dialog.h"
#include "ui/parameter_set.h"
#include "network/udp.h"
#include "ui_window.h"
#include "ui/window.h"
#include "utility/utility.h"

namespace biped
{
namespace ground_station
{
Window::Window(QWidget *parent) : QWidget(parent), controller_parameter_initialized_(false), parameter_set_pinned_count_(0), planner_parameter_initialized_(false), rendering_fps_cap_biped_message_(UIParameter::window_rendering_fps_cap_default_biped_message), rendering_fps_cap_camera_frame_(UIParameter::window_rendering_fps_cap_default_camera_frame)
{
    biped_message_.controller_parameter.pid_controller_saturation_position_x.input_lower = 0;
    biped_message_.controller_parameter.pid_controller_saturation_position_x.input_upper = 0;
    biped_message_render_ = biped_message_;

    ui_ = std::make_unique<Ui::Window>();

    if (!ui_)
    {
        return;
    }

    ui_->setupUi(this);

    show();
    initialize();
}

Window::~Window()
{
    udp_camera_->close();
    udp_biped_message_->close();

    daemon_camera_->stop();
    daemon_inbound_->stop();
    daemon_joypad_->stop();
    daemon_logging_->stop();

    thread_daemon_camera_->quit();
    thread_daemon_inbound_->quit();
    thread_daemon_joypad_->quit();
    thread_daemon_logging_->quit();
    thread_daemon_outbound_->quit();

    thread_daemon_camera_->wait();
    thread_daemon_inbound_->wait();
    thread_daemon_joypad_->wait();
    thread_daemon_logging_->wait();
    thread_daemon_outbound_->wait();
}

void
Window::logToStatusBar(const std::string& status)
{
    std::lock_guard<std::mutex> lock(mutex_status_bar_);
    ui_->label_status->setText(QString::fromStdString(status));
}

void
Window::setRenderingFPSCapBipedMessage(const unsigned& rendering_fps_cap_biped_message)
{
    rendering_fps_cap_biped_message_ = rendering_fps_cap_biped_message;
}

void
Window::setRenderingFPSCapCameraFrame(const unsigned& rendering_fps_cap_camera_frame)
{
    rendering_fps_cap_camera_frame_ = rendering_fps_cap_camera_frame;
}

void
Window::onCameraDaemonFrameReceived(const QImage& frame)
{
    renderCameraFrame(frame);
}

void
Window::onControllerInputPushButtonApplyClicked()
{
    ConfirmDialog confirm_dialog;

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Send", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to apply and send the controller parameters to Biped at \"" + ip_biped_ + "\"?"));
    confirm_dialog.setTitle("Apply Controller Parameters");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_biped_message_);
    const biped::firmware::BipedMessage message_current = biped_message_;
    lock.unlock();

    biped::firmware::BipedMessage message;

    message.controller_parameter.pid_controller_gain_attitude_y.proportional = ui_->controller_parameter_balance_double_spin_box_proportional->value();
    message.controller_parameter.pid_controller_gain_attitude_y.integral = ui_->controller_parameter_balance_double_spin_box_integral->value();
    message.controller_parameter.pid_controller_gain_attitude_y.differential = ui_->controller_parameter_balance_double_spin_box_differential->value();
    message.controller_parameter.pid_controller_gain_attitude_y.integral_max = ui_->controller_parameter_balance_double_spin_box_integral_max->value();

    message.controller_parameter.pid_controller_gain_position_x.proportional = ui_->controller_parameter_forward_double_spin_box_proportional->value();
    message.controller_parameter.pid_controller_gain_position_x.integral = ui_->controller_parameter_forward_double_spin_box_integral->value();
    message.controller_parameter.pid_controller_gain_position_x.differential = ui_->controller_parameter_forward_double_spin_box_differential->value();
    message.controller_parameter.pid_controller_gain_position_x.integral_max = ui_->controller_parameter_forward_double_spin_box_integral_max->value();
    message.controller_parameter.pid_controller_saturation_position_x.input_upper = ui_->controller_parameter_forward_double_spin_box_input_upper->value();
    message.controller_parameter.pid_controller_saturation_position_x.input_lower = ui_->controller_parameter_forward_double_spin_box_input_lower->value();

    message.controller_parameter.pid_controller_gain_attitude_z.proportional = ui_->controller_parameter_turning_double_spin_box_proportional->value();
    message.controller_parameter.pid_controller_gain_attitude_z.integral = ui_->controller_parameter_turning_double_spin_box_integral->value();
    message.controller_parameter.pid_controller_gain_attitude_z.differential = ui_->controller_parameter_turning_double_spin_box_differential->value();
    message.controller_parameter.pid_controller_gain_attitude_z.integral_max = ui_->controller_parameter_turning_double_spin_box_integral_max->value();
    message.controller_parameter.attitude_z_gain_open_loop = ui_->controller_parameter_turning_double_spin_box_open_loop->value();

    message.controller_reference.attitude_y = message_current.controller_reference.attitude_y;
    message.controller_reference.attitude_z = message_current.controller_reference.attitude_z;
    message.controller_reference.position_x = message_current.controller_reference.position_x;

    logToStatusBar("Sending controller parameters...");

    emit operateOutboundDaemon(message);
}

void
Window::onControllerInputPushButtonRevertClicked()
{
    ConfirmDialog confirm_dialog;

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Revert", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to revert the entered controller parameters back to the current values?"));
    confirm_dialog.setTitle("Revert Controller Parameters");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_biped_message_);
    const biped::firmware::BipedMessage message = biped_message_;
    lock.unlock();

    ui_->controller_parameter_balance_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_attitude_y.proportional);
    ui_->controller_parameter_balance_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_attitude_y.integral);
    ui_->controller_parameter_balance_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_attitude_y.differential);
    ui_->controller_parameter_balance_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_attitude_y.integral_max);

    ui_->controller_parameter_forward_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_position_x.proportional);
    ui_->controller_parameter_forward_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_position_x.integral);
    ui_->controller_parameter_forward_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_position_x.differential);
    ui_->controller_parameter_forward_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_position_x.integral_max);
    ui_->controller_parameter_forward_double_spin_box_input_upper->setValue(message.controller_parameter.pid_controller_saturation_position_x.input_upper);
    ui_->controller_parameter_forward_double_spin_box_input_lower->setValue(message.controller_parameter.pid_controller_saturation_position_x.input_lower);

    ui_->controller_parameter_turning_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_attitude_z.proportional);
    ui_->controller_parameter_turning_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_attitude_z.integral);
    ui_->controller_parameter_turning_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_attitude_z.differential);
    ui_->controller_parameter_turning_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_attitude_z.integral_max);
    ui_->controller_parameter_turning_double_spin_box_open_loop->setValue(message.controller_parameter.attitude_z_gain_open_loop);

    logToStatusBar("Reverted entered controller parameters to the current values.");
}

void
Window::onControllerInputPushButtonSaveClicked()
{
    ParameterDialog parameter_dialog;
    std::string parameter_set_name = "Parameter Set " + std::to_string(parameter_sets_.size() + 1);
    QPushButton* push_button_cancel = parameter_dialog.addButton("Cancel", QDialogButtonBox::NoRole);

    parameter_dialog.addButton("Save", QDialogButtonBox::DestructiveRole);
    parameter_dialog.setDefaultButton(push_button_cancel);
    parameter_dialog.setName(QString::fromStdString(parameter_set_name));
    parameter_dialog.setTitle("Save Controller Parameters");
    parameter_dialog.exec();

    if (parameter_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    parameter_set_name = parameter_dialog.name().toStdString();

    biped::firmware::BipedMessage message;
    ParameterSet* parameter_set = new ParameterSet(ui_->parameters_scroll_area_widget);
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream time_ss;

    message.controller_parameter.pid_controller_gain_attitude_y.proportional = ui_->controller_parameter_balance_double_spin_box_proportional->value();
    message.controller_parameter.pid_controller_gain_attitude_y.integral = ui_->controller_parameter_balance_double_spin_box_integral->value();
    message.controller_parameter.pid_controller_gain_attitude_y.differential = ui_->controller_parameter_balance_double_spin_box_differential->value();
    message.controller_parameter.pid_controller_gain_attitude_y.integral_max = ui_->controller_parameter_balance_double_spin_box_integral_max->value();

    message.controller_parameter.pid_controller_gain_position_x.proportional = ui_->controller_parameter_forward_double_spin_box_proportional->value();
    message.controller_parameter.pid_controller_gain_position_x.integral = ui_->controller_parameter_forward_double_spin_box_integral->value();
    message.controller_parameter.pid_controller_gain_position_x.differential = ui_->controller_parameter_forward_double_spin_box_differential->value();
    message.controller_parameter.pid_controller_gain_position_x.integral_max = ui_->controller_parameter_forward_double_spin_box_integral_max->value();
    message.controller_parameter.pid_controller_saturation_position_x.input_upper = ui_->controller_parameter_forward_double_spin_box_input_upper->value();
    message.controller_parameter.pid_controller_saturation_position_x.input_lower = ui_->controller_parameter_forward_double_spin_box_input_lower->value();

    message.controller_parameter.pid_controller_gain_attitude_z.proportional = ui_->controller_parameter_turning_double_spin_box_proportional->value();
    message.controller_parameter.pid_controller_gain_attitude_z.integral = ui_->controller_parameter_turning_double_spin_box_integral->value();
    message.controller_parameter.pid_controller_gain_attitude_z.differential = ui_->controller_parameter_turning_double_spin_box_differential->value();
    message.controller_parameter.pid_controller_gain_attitude_z.integral_max = ui_->controller_parameter_turning_double_spin_box_integral_max->value();
    message.controller_parameter.attitude_z_gain_open_loop = ui_->controller_parameter_turning_double_spin_box_open_loop->value();

    time_ss << std::put_time(std::localtime(&time), "%m/%d/%Y %X");

    parameter_set->setBipedMessage(message);
    parameter_set->setDateTime(time_ss.str());
    parameter_set->setIndex(parameter_sets_.size());
    parameter_set->setName(parameter_set_name);
    parameter_set->setNotes(parameter_dialog.notes().toStdString());

    connect(parameter_set, &ParameterSet::pushButtonDeleteClicked, this, &Window::onParameterSetPushButtonDeleteClicked);
    connect(parameter_set, &ParameterSet::pushButtonLoadClicked, this, &Window::onParameterSetPushButtonLoadClicked);
    connect(parameter_set, &ParameterSet::pushButtonNotesClicked, this, &Window::onParameterSetPushButtonNotesClicked);
    connect(parameter_set, &ParameterSet::pushButtonPinClicked, this, &Window::onParameterSetPushButtonPinClicked);

    ui_->parameters_input_push_button_delete_all->setEnabled(true);
    ui_->parameters_scroll_area_widget_layout_parameter_set->addWidget(parameter_set);
    ui_->parameters_scroll_area_widget_label_empty->setVisible(false);

    parameter_sets_.push_back(parameter_set);

    logToStatusBar("Saved current controller parameters as \"" + parameter_set_name + "\".");
}

void
Window::onControllerResponsePlotPushButtonZoomToFitBalanceClicked()
{
    ui_->controller_response_plot_balance->zoomToFit();
}

void
Window::onControllerResponsePlotPushButtonZoomToFitForwardClicked()
{
    ui_->controller_response_plot_forward->zoomToFit();
}

void
Window::onControllerResponsePlotPushButtonZoomToFitTurningClicked()
{
    ui_->controller_response_plot_turning->zoomToFit();
}

void
Window::onInboundDaemonMessageReceived(const biped::firmware::BipedMessage& message)
{
    std::unique_lock<std::mutex> lock(mutex_biped_message_);
    biped_message_ = message;
    lock.unlock();

    renderBipedMessage(message);
}

void
Window::onParametersInputPushButtonDeleteAllClicked()
{
    ConfirmDialog confirm_dialog;

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Delete", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to delete all saved controller parameters?"));
    confirm_dialog.setTitle("Delete Saved Parameters");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    for (ParameterSet* parameter_set : parameter_sets_)
    {
        if (parameter_set)
        {
            disconnect(parameter_set, &ParameterSet::pushButtonDeleteClicked, this, &Window::onParameterSetPushButtonDeleteClicked);
            disconnect(parameter_set, &ParameterSet::pushButtonLoadClicked, this, &Window::onParameterSetPushButtonLoadClicked);
            disconnect(parameter_set, &ParameterSet::pushButtonNotesClicked, this, &Window::onParameterSetPushButtonNotesClicked);
            disconnect(parameter_set, &ParameterSet::pushButtonPinClicked, this, &Window::onParameterSetPushButtonPinClicked);

            delete parameter_set;
            parameter_set = nullptr;
        }
    }

    parameter_sets_.clear();
    parameter_set_pinned_count_ = 0;

    ui_->parameters_input_push_button_delete_all->setEnabled(false);
    ui_->parameters_scroll_area_widget_label_empty->setVisible(true);

    logToStatusBar("Deleted all saved controller parameters.");
}

void
Window::onParameterSetPushButtonDeleteClicked(const size_t& parameter_set_index)
{
    if (parameter_set_index >= parameter_sets_.size() || !parameter_sets_[parameter_set_index])
    {
        logToStatusBar("Failed to delete saved parameters.");
        return;
    }

    ConfirmDialog confirm_dialog;
    const std::string parameter_set_name = parameter_sets_[parameter_set_index]->getName();

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Delete", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to delete \"" + parameter_set_name + "\"?"));
    confirm_dialog.setTitle("Delete \"" + QString::fromStdString(parameter_set_name) + "\"");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    if (parameter_sets_[parameter_set_index]->getPinned())
    {
        parameter_set_pinned_count_ --;
    }

    disconnect(parameter_sets_[parameter_set_index], &ParameterSet::pushButtonDeleteClicked, this, &Window::onParameterSetPushButtonDeleteClicked);
    disconnect(parameter_sets_[parameter_set_index], &ParameterSet::pushButtonLoadClicked, this, &Window::onParameterSetPushButtonLoadClicked);
    disconnect(parameter_sets_[parameter_set_index], &ParameterSet::pushButtonNotesClicked, this, &Window::onParameterSetPushButtonNotesClicked);
    disconnect(parameter_sets_[parameter_set_index], &ParameterSet::pushButtonPinClicked, this, &Window::onParameterSetPushButtonPinClicked);

    delete parameter_sets_[parameter_set_index];
    parameter_sets_.erase(parameter_sets_.begin() + parameter_set_index);

    if (parameter_sets_.size() == 0)
    {
        ui_->parameters_input_push_button_delete_all->setEnabled(false);
        ui_->parameters_scroll_area_widget_label_empty->setVisible(true);
    }
    else
    {
        for (size_t i = 0; i < parameter_sets_.size(); i ++)
        {
            if (parameter_sets_[i])
            {
                parameter_sets_[i]->setIndex(i);
            }
        }
    }

    logToStatusBar("Deleted \"" + parameter_set_name + "\".");
}

void
Window::onParameterSetPushButtonLoadClicked(const size_t& parameter_set_index)
{
    if (parameter_set_index >= parameter_sets_.size() || !parameter_sets_[parameter_set_index])
    {
        logToStatusBar("Failed to load saved parameters.");
        return;
    }

    ConfirmDialog confirm_dialog;
    const std::string parameter_set_name = parameter_sets_[parameter_set_index]->getName();

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Load", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to load \"" + parameter_set_name + "\"?"));
    confirm_dialog.setTitle("Load \"" + QString::fromStdString(parameter_set_name) + "\"");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    const biped::firmware::BipedMessage message = parameter_sets_[parameter_set_index]->getBipedMessage();

    ui_->controller_parameter_balance_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_attitude_y.proportional);
    ui_->controller_parameter_balance_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_attitude_y.integral);
    ui_->controller_parameter_balance_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_attitude_y.differential);
    ui_->controller_parameter_balance_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_attitude_y.integral_max);

    ui_->controller_parameter_forward_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_position_x.proportional);
    ui_->controller_parameter_forward_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_position_x.integral);
    ui_->controller_parameter_forward_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_position_x.differential);
    ui_->controller_parameter_forward_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_position_x.integral_max);
    ui_->controller_parameter_forward_double_spin_box_input_upper->setValue(message.controller_parameter.pid_controller_saturation_position_x.input_upper);
    ui_->controller_parameter_forward_double_spin_box_input_lower->setValue(message.controller_parameter.pid_controller_saturation_position_x.input_lower);

    ui_->controller_parameter_turning_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_attitude_z.proportional);
    ui_->controller_parameter_turning_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_attitude_z.integral);
    ui_->controller_parameter_turning_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_attitude_z.differential);
    ui_->controller_parameter_turning_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_attitude_z.integral_max);
    ui_->controller_parameter_turning_double_spin_box_open_loop->setValue(message.controller_parameter.attitude_z_gain_open_loop);

    ui_->tab_widget->setCurrentIndex(0);

    logToStatusBar("Loaded \"" + parameter_set_name + "\".");
}

void
Window::onParameterSetPushButtonNotesClicked(const size_t& parameter_set_index)
{
    if (parameter_set_index >= parameter_sets_.size() || !parameter_sets_[parameter_set_index])
    {
        logToStatusBar("Failed to access notes.");
        return;
    }

    ParameterDialog parameter_dialog;
    QPushButton* push_button_cancel = parameter_dialog.addButton("Cancel", QDialogButtonBox::NoRole);

    parameter_dialog.addButton("Save", QDialogButtonBox::DestructiveRole);
    parameter_dialog.setDefaultButton(push_button_cancel);
    parameter_dialog.setName(QString::fromStdString(parameter_sets_[parameter_set_index]->getName()));
    parameter_dialog.setNotes(QString::fromStdString(parameter_sets_[parameter_set_index]->getNotes()));
    parameter_dialog.setTitle(QString::fromStdString(parameter_sets_[parameter_set_index]->getName()));
    parameter_dialog.exec();

    if (parameter_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    if (parameter_sets_[parameter_set_index]->getName() != parameter_dialog.name().toStdString())
    {
        logToStatusBar("Renamed \"" + parameter_sets_[parameter_set_index]->getName() + "\" to \"" + parameter_dialog.name().toStdString() + "\".");
        parameter_sets_[parameter_set_index]->setName(parameter_dialog.name().toStdString());
    }

    if (parameter_sets_[parameter_set_index]->getNotes() != parameter_dialog.notes().toStdString())
    {
        parameter_sets_[parameter_set_index]->setNotes(parameter_dialog.notes().toStdString());
        logToStatusBar("Updated notes for \"" + parameter_sets_[parameter_set_index]->getName() + "\".");
    }
}

void
Window::onParameterSetPushButtonPinClicked(const size_t& parameter_set_index)
{
    if (parameter_set_index >= parameter_sets_.size() || !parameter_sets_[parameter_set_index])
    {
        logToStatusBar("Failed to pin saved parameters.");
        return;
    }

    ParameterSet* parameter_set = parameter_sets_[parameter_set_index];

    if (parameter_set->getPinned())
    {
        ui_->parameters_scroll_area_widget_layout_parameter_set->removeWidget(parameter_set);
        ui_->parameters_scroll_area_widget_layout_parameter_set->insertWidget(0, parameter_set);

        parameter_sets_.erase(parameter_sets_.begin() + parameter_set_index);
        parameter_sets_.insert(parameter_sets_.begin(), parameter_set);

        for (size_t i = 0; i < parameter_sets_.size(); i ++)
        {
            if (parameter_sets_[i])
            {
                parameter_sets_[i]->setIndex(i);
            }
        }

        parameter_set_pinned_count_ ++;

        logToStatusBar("Pinned \"" + parameter_set->getName() + "\".");
    }
    else
    {
        parameter_set_pinned_count_ --;

        ui_->parameters_scroll_area_widget_layout_parameter_set->removeWidget(parameter_set);
        ui_->parameters_scroll_area_widget_layout_parameter_set->insertWidget(parameter_set_pinned_count_, parameter_set);

        parameter_sets_.erase(parameter_sets_.begin() + parameter_set_index);
        parameter_sets_.insert(parameter_sets_.begin() + parameter_set_pinned_count_, parameter_set);

        for (size_t i = 0; i < parameter_sets_.size(); i ++)
        {
            if (parameter_sets_[i])
            {
                parameter_sets_[i]->setIndex(i);
            }
        }

        logToStatusBar("Unpinned \"" + parameter_set->getName() + "\".");
    }
}

void
Window::onPlannerJoyPadXChanged(const double& value)
{
}

void
Window::onPlannerJoyPadYChanged(const double& value)
{
}

void
Window::onPlannerParameterPushButtonApplyClicked()
{
    ConfirmDialog confirm_dialog;

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Send", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to apply and send the planner parameters to Biped at \"" + ip_biped_ + "\"?"));
    confirm_dialog.setTitle("Apply Planner Parameters");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_biped_message_);
    const biped::firmware::BipedMessage message_current = biped_message_;
    lock.unlock();

    biped::firmware::BipedMessage message;

    message.controller_parameter.pid_controller_gain_attitude_y.proportional = message_current.controller_parameter.pid_controller_gain_attitude_y.proportional;
    message.controller_parameter.pid_controller_gain_attitude_y.integral = message_current.controller_parameter.pid_controller_gain_attitude_y.integral;
    message.controller_parameter.pid_controller_gain_attitude_y.differential = message_current.controller_parameter.pid_controller_gain_attitude_y.differential;
    message.controller_parameter.pid_controller_gain_attitude_y.integral_max = message_current.controller_parameter.pid_controller_gain_attitude_y.integral_max;

    message.controller_parameter.pid_controller_gain_position_x.proportional = message_current.controller_parameter.pid_controller_gain_position_x.proportional;
    message.controller_parameter.pid_controller_gain_position_x.integral = message_current.controller_parameter.pid_controller_gain_position_x.integral;
    message.controller_parameter.pid_controller_gain_position_x.differential = message_current.controller_parameter.pid_controller_gain_position_x.differential;
    message.controller_parameter.pid_controller_gain_position_x.integral_max = message_current.controller_parameter.pid_controller_gain_position_x.integral_max;
    message.controller_parameter.pid_controller_saturation_position_x.input_upper = message_current.controller_parameter.pid_controller_saturation_position_x.input_upper;
    message.controller_parameter.pid_controller_saturation_position_x.input_lower = message_current.controller_parameter.pid_controller_saturation_position_x.input_lower;

    message.controller_parameter.pid_controller_gain_attitude_z.proportional = message_current.controller_parameter.pid_controller_gain_attitude_z.proportional;
    message.controller_parameter.pid_controller_gain_attitude_z.integral = message_current.controller_parameter.pid_controller_gain_attitude_z.integral;
    message.controller_parameter.pid_controller_gain_attitude_z.differential = message_current.controller_parameter.pid_controller_gain_attitude_z.differential;
    message.controller_parameter.pid_controller_gain_attitude_z.integral_max = message_current.controller_parameter.pid_controller_gain_attitude_z.integral_max;
    message.controller_parameter.attitude_z_gain_open_loop =  message_current.controller_parameter.attitude_z_gain_open_loop;

    message.controller_reference.attitude_y = message_current.controller_reference.attitude_y;
    message.controller_reference.attitude_z = degreesToRadians(ui_->planner_parameter_double_spin_box_turning->value());
    message.controller_reference.position_x = ui_->planner_parameter_double_spin_box_forward->value();

    logToStatusBar("Sending planner parameters...");

    emit operateOutboundDaemon(message);
}

void
Window::onPlannerParameterPushButtonRevertClicked()
{
    ConfirmDialog confirm_dialog;

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Revert", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText(QString::fromStdString("Are you sure you want to revert the entered planner parameters back to the current values?"));
    confirm_dialog.setTitle("Revert Planner Parameters");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    std::unique_lock<std::mutex> lock(mutex_biped_message_);
    const biped::firmware::BipedMessage message = biped_message_;
    lock.unlock();

    ui_->planner_parameter_double_spin_box_forward->setValue(message.controller_reference.position_x);
    ui_->planner_parameter_double_spin_box_turning->setValue(radiansToDegrees(message.controller_reference.attitude_z));

    logToStatusBar("Reverted entered planner parameters to the current values.");
}

void
Window::onSettingsBipedIPAddressLineEditEditingFinished()
{
    ui_->settings_biped_ip_address_line_edit->clearFocus();
}

void
Window::onSettingsBipedIPAddressPushButtonApplyClicked()
{
    ip_biped_ = ui_->settings_biped_ip_address_line_edit->text().toStdString();

    logToStatusBar("Communicating with Biped at \"" + ip_biped_ + "\".");
}

void
Window::onSettingsLoggingPushButtonStartClicked()
{
    daemon_logging_->start();

    logToStatusBar("Started logging.");

    ui_->settings_logging_push_button_start->setEnabled(false);
    ui_->settings_logging_push_button_stop->setEnabled(true);
}

void
Window::onSettingsLoggingPushButtonStopClicked()
{
    daemon_logging_->stop();

    logToStatusBar("Stopped logging.");

    ui_->settings_logging_push_button_start->setEnabled(true);
    ui_->settings_logging_push_button_stop->setEnabled(false);
}

void
Window::onSettingsThemePushButtonLoadClicked()
{
    std::filesystem::path path_file_name_load = QFileDialog::getOpenFileName(this, "Load Theme", tr(std::getenv("HOME")), tr("*.qss"), nullptr, QFileDialog::DontUseNativeDialog).toStdString();
    QFile file_style(path_file_name_load);

    if (path_file_name_load == "")
    {
        return;
    }

    file_style.open(QFile::ReadOnly);
    qApp->setStyleSheet(QString(file_style.readAll()));
    ui_->settings_theme_label_value->setText(QString::fromStdString(path_file_name_load.stem()));
    ui_->settings_theme_push_button_reset->setEnabled(true);

    logToStatusBar("Loaded theme \"" + path_file_name_load.stem().string() + "\".");
}

void
Window::onSettingsThemePushButtonResetClicked()
{
    ConfirmDialog confirm_dialog;

    QPushButton* push_button_cancel = confirm_dialog.addButton("Cancel", QDialogButtonBox::NoRole);
    confirm_dialog.addButton("Reset", QDialogButtonBox::DestructiveRole);
    confirm_dialog.setDefaultButton(push_button_cancel);
    confirm_dialog.setInformativeText("You cannot undo this action.");
    confirm_dialog.setText("Are you sure you want to reset to default theme?");
    confirm_dialog.setTitle("Reset To Default Theme");
    confirm_dialog.exec();

    if (confirm_dialog.clickedButton() == push_button_cancel)
    {
        logToStatusBar("");
        return;
    }

    qApp->setStyleSheet("");
    ui_->settings_theme_label_value->setText("Default");
    ui_->settings_theme_push_button_reset->setEnabled(false);

    logToStatusBar("Reset to default theme.");
}

void
Window::connectSignals()
{
    disconnectSignals();

    connect(daemon_camera_.get(), &CameraDaemon::frameReceived, this, &Window::onCameraDaemonFrameReceived);
    connect(daemon_inbound_.get(), &InboundDaemon::messageReceived, this, &Window::onInboundDaemonMessageReceived);
    connect(daemon_joypad_.get(), &JoypadDaemon::xChanged, ui_->planner_joypad, &JoyPad::setX);
    connect(daemon_joypad_.get(), &JoypadDaemon::yChanged, ui_->planner_joypad, &JoyPad::setY);
    connect(ui_->controller_input_push_button_apply, &QPushButton::clicked, this, &Window::onControllerInputPushButtonApplyClicked);
    connect(ui_->controller_input_push_button_revert, &QPushButton::clicked, this, &Window::onControllerInputPushButtonRevertClicked);
    connect(ui_->controller_input_push_button_save, &QPushButton::clicked, this, &Window::onControllerInputPushButtonSaveClicked);
    connect(ui_->controller_response_plot_push_button_zoom_to_fit_balance, &QPushButton::clicked, this, &Window::onControllerResponsePlotPushButtonZoomToFitBalanceClicked);
    connect(ui_->controller_response_plot_push_button_zoom_to_fit_forward, &QPushButton::clicked, this, &Window::onControllerResponsePlotPushButtonZoomToFitForwardClicked);
    connect(ui_->controller_response_plot_push_button_zoom_to_fit_turning, &QPushButton::clicked, this, &Window::onControllerResponsePlotPushButtonZoomToFitTurningClicked);
    connect(ui_->parameters_input_push_button_delete_all, &QPushButton::clicked, this, &Window::onParametersInputPushButtonDeleteAllClicked);
    connect(ui_->planner_joypad, &JoyPad::xChanged, this, &Window::onPlannerJoyPadXChanged);
    connect(ui_->planner_joypad, &JoyPad::yChanged, this, &Window::onPlannerJoyPadYChanged);
    connect(ui_->planner_parameter_push_button_apply, &QPushButton::clicked, this, &Window::onPlannerParameterPushButtonApplyClicked);
    connect(ui_->planner_parameter_push_button_revert, &QPushButton::clicked, this, &Window::onPlannerParameterPushButtonRevertClicked);
    connect(ui_->settings_biped_ip_address_line_edit, &QLineEdit::editingFinished, this, &Window::onSettingsBipedIPAddressLineEditEditingFinished);
    connect(ui_->settings_biped_ip_address_push_button_apply, &QPushButton::clicked, this, &Window::onSettingsBipedIPAddressPushButtonApplyClicked);
    connect(ui_->settings_logging_push_button_start, &QPushButton::clicked, this, &Window::onSettingsLoggingPushButtonStartClicked);
    connect(ui_->settings_logging_push_button_stop, &QPushButton::clicked, this, &Window::onSettingsLoggingPushButtonStopClicked);
    connect(ui_->settings_theme_push_button_load, &QPushButton::clicked, this, &Window::onSettingsThemePushButtonLoadClicked);
    connect(ui_->settings_theme_push_button_reset, &QPushButton::clicked, this, &Window::onSettingsThemePushButtonResetClicked);
}

void
Window::disconnectSignals()
{
    disconnect(daemon_camera_.get(), &CameraDaemon::frameReceived, this, &Window::onCameraDaemonFrameReceived);
    disconnect(daemon_inbound_.get(), &InboundDaemon::messageReceived, this, &Window::onInboundDaemonMessageReceived);
    disconnect(daemon_joypad_.get(), &JoypadDaemon::xChanged, ui_->planner_joypad, &JoyPad::setX);
    disconnect(daemon_joypad_.get(), &JoypadDaemon::yChanged, ui_->planner_joypad, &JoyPad::setY);
    disconnect(ui_->controller_input_push_button_apply, &QPushButton::clicked, this, &Window::onControllerInputPushButtonApplyClicked);
    disconnect(ui_->controller_input_push_button_revert, &QPushButton::clicked, this, &Window::onControllerInputPushButtonRevertClicked);
    disconnect(ui_->controller_input_push_button_save, &QPushButton::clicked, this, &Window::onControllerInputPushButtonSaveClicked);
    disconnect(ui_->controller_response_plot_push_button_zoom_to_fit_balance, &QPushButton::clicked, this, &Window::onControllerResponsePlotPushButtonZoomToFitBalanceClicked);
    disconnect(ui_->controller_response_plot_push_button_zoom_to_fit_forward, &QPushButton::clicked, this, &Window::onControllerResponsePlotPushButtonZoomToFitForwardClicked);
    disconnect(ui_->controller_response_plot_push_button_zoom_to_fit_turning, &QPushButton::clicked, this, &Window::onControllerResponsePlotPushButtonZoomToFitTurningClicked);
    disconnect(ui_->parameters_input_push_button_delete_all, &QPushButton::clicked, this, &Window::onParametersInputPushButtonDeleteAllClicked);
    disconnect(ui_->planner_joypad, &JoyPad::xChanged, this, &Window::onPlannerJoyPadXChanged);
    disconnect(ui_->planner_joypad, &JoyPad::yChanged, this, &Window::onPlannerJoyPadYChanged);
    disconnect(ui_->planner_parameter_push_button_apply, &QPushButton::clicked, this, &Window::onPlannerParameterPushButtonApplyClicked);
    disconnect(ui_->planner_parameter_push_button_revert, &QPushButton::clicked, this, &Window::onPlannerParameterPushButtonRevertClicked);
    disconnect(ui_->settings_biped_ip_address_line_edit, &QLineEdit::editingFinished, this, &Window::onSettingsBipedIPAddressLineEditEditingFinished);
    disconnect(ui_->settings_biped_ip_address_push_button_apply, &QPushButton::clicked, this, &Window::onSettingsBipedIPAddressPushButtonApplyClicked);
    disconnect(ui_->settings_logging_push_button_start, &QPushButton::clicked, this, &Window::onSettingsLoggingPushButtonStartClicked);
    disconnect(ui_->settings_logging_push_button_stop, &QPushButton::clicked, this, &Window::onSettingsLoggingPushButtonStopClicked);
    disconnect(ui_->settings_theme_push_button_load, &QPushButton::clicked, this, &Window::onSettingsThemePushButtonLoadClicked);
    disconnect(ui_->settings_theme_push_button_reset, &QPushButton::clicked, this, &Window::onSettingsThemePushButtonResetClicked);
}

void
Window::initialize()
{
    std::vector<QPen> controller_response_plot_pens = {QPen(Qt::red, 2), QPen(Qt::green, 2)};
    QString regular_expression_ip_address_octet = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegularExpression regular_expression_ip_address("^" + regular_expression_ip_address_octet + "(\\." + regular_expression_ip_address_octet + ")" + "(\\." + regular_expression_ip_address_octet + ")" + "(\\." + regular_expression_ip_address_octet + ")$");
    QRegularExpressionValidator *validator_ip_address = new QRegularExpressionValidator(regular_expression_ip_address, this);

    ui_->controller_response_plot_balance->addCurves(controller_response_plot_pens);
    ui_->controller_response_plot_balance->xAxis->grid()->setVisible(false);
    ui_->controller_response_plot_balance->xAxis->setTickLabels(false);
    ui_->controller_response_plot_balance->xAxis->setTicks(false);
    ui_->controller_response_plot_forward->addCurves(controller_response_plot_pens);
    ui_->controller_response_plot_forward->xAxis->grid()->setVisible(false);
    ui_->controller_response_plot_forward->xAxis->setTickLabels(false);
    ui_->controller_response_plot_forward->xAxis->setTicks(false);
    ui_->controller_response_plot_turning->addCurves(controller_response_plot_pens);
    ui_->controller_response_plot_turning->xAxis->grid()->setVisible(false);
    ui_->controller_response_plot_turning->xAxis->setTickLabels(false);
    ui_->controller_response_plot_turning->xAxis->setTicks(false);
    ui_->settings_biped_ip_address_line_edit->setValidator(validator_ip_address);
    ui_->settings_biped_ip_address_line_edit->setText(QString::fromStdString(ip_biped_));
    ui_->settings_logging_push_button_stop->setEnabled(false);
    ui_->settings_theme_push_button_reset->setEnabled(false);

    if (parameter_sets_.size() == 0)
    {
        ui_->parameters_input_push_button_delete_all->setEnabled(false);
        ui_->parameters_scroll_area_widget_label_empty->setVisible(true);
    }

    time_point_last_render_biped_message_ = std::chrono::system_clock::now();
    time_point_last_render_camera_frame_ = std::chrono::system_clock::now();

    if (!udp_biped_message_ || !udp_biped_message_->bound())
    {
        logToStatusBar("Failed to initialize Biped message UDP.");
        return;
    }

    if (!udp_camera_ || !udp_camera_->bound())
    {
        logToStatusBar("Failed to initialize camera UDP.");
        return;
    }

    logToStatusBar("Communicating with Biped at \"" + ip_biped_ + "\".");

    if (daemon_camera_)
    {
        thread_daemon_camera_ = std::make_unique<QThread>();

        connect(this, &Window::operateCameraDaemon, daemon_camera_.get(), &CameraDaemon::operate);

        daemon_camera_->moveToThread(thread_daemon_camera_.get());
        daemon_camera_->start();
        thread_daemon_camera_->start();

        emit operateCameraDaemon();
    }
    else
    {
        logToStatusBar("Failed to start camera daemon.");
    }

    if (daemon_inbound_)
    {
        thread_daemon_inbound_ = std::make_unique<QThread>();

        connect(this, &Window::operateInboundDaemon, daemon_inbound_.get(), &InboundDaemon::operate);

        daemon_inbound_->moveToThread(thread_daemon_inbound_.get());
        daemon_inbound_->start();
        thread_daemon_inbound_->start();

        emit operateInboundDaemon();
    }
    else
    {
        logToStatusBar("Failed to start inbound daemon.");
    }

    if (daemon_joypad_)
    {
        thread_daemon_joypad_ = std::make_unique<QThread>();

        connect(this, &Window::operateJoypadDaemon, daemon_joypad_.get(), &JoypadDaemon::operate);

        daemon_joypad_->moveToThread(thread_daemon_joypad_.get());
        daemon_joypad_->start();
        thread_daemon_joypad_->start();

        emit operateJoypadDaemon();
    }
    else
    {
        logToStatusBar("Failed to start joypad daemon.");
    }

    if (daemon_logging_)
    {
        thread_daemon_logging_ = std::make_unique<QThread>();

        connect(daemon_camera_.get(), &CameraDaemon::frameReceived, daemon_logging_.get(), &LoggingDaemon::onCameraDaemonFrameReceived);
        connect(daemon_inbound_.get(), &InboundDaemon::messageReceived, daemon_logging_.get(), &LoggingDaemon::onInboundDaemonMessageReceived);

        daemon_logging_->moveToThread(thread_daemon_logging_.get());
        thread_daemon_logging_->start();
    }
    else
    {
        logToStatusBar("Failed to start logging daemon.");
    }

    if (daemon_outbound_)
    {
        thread_daemon_outbound_ = std::make_unique<QThread>();

        connect(this, &Window::operateOutboundDaemon, daemon_outbound_.get(), &OutboundDaemon::operate);

        daemon_outbound_->moveToThread(thread_daemon_outbound_.get());
        thread_daemon_outbound_->start();
    }
    else
    {
        logToStatusBar("Failed to initialize outbound daemon.");
    }

    connectSignals();
}

void
Window::renderCameraFrame(const QImage& frame)
{
    if (rendering_fps_cap_camera_frame_ == 0)
    {
        return;
    }

    const unsigned long duration_since_last_render = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_point_last_render_camera_frame_).count();

    if (duration_since_last_render < fpsToMilliseconds(rendering_fps_cap_camera_frame_))
    {
        return;
    }

    ui_->camera_view->onCameraFrame(frame);

    time_point_last_render_camera_frame_ = std::chrono::system_clock::now();
}

void
Window::renderBipedMessage(const biped::firmware::BipedMessage& message)
{
    if (rendering_fps_cap_biped_message_ == 0)
    {
        return;
    }

    const unsigned long duration_since_last_render = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_point_last_render_biped_message_).count();

    if (duration_since_last_render < fpsToMilliseconds(rendering_fps_cap_biped_message_))
    {
        return;
    }

    bool controller_parameter_initialized = controller_parameter_initialized_;
    bool controller_parameter_updated = false;
    const unsigned long duration_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    bool planner_parameter_initialized = planner_parameter_initialized_;
    bool planner_parameter_updated = false;

    ui_->data_header_label_value_sequence->setText(QString::number(message.sequence));
    ui_->data_header_label_value_timestamp->setText(QString::number(message.timestamp));

    if (message.actuation_command.motor_enable)
    {
        ui_->data_actuation_command_label_value_motor_enable->setText("True");
    }
    else
    {
        ui_->data_actuation_command_label_value_motor_enable->setText("False");
    }

    if (message.actuation_command.motor_left_forward)
    {
        ui_->data_actuation_command_label_value_motor_left_forward->setText("True");
    }
    else
    {
        ui_->data_actuation_command_label_value_motor_left_forward->setText("False");
    }

    if (message.actuation_command.motor_right_forward)
    {
        ui_->data_actuation_command_label_value_motor_right_forward->setText("True");
    }
    else
    {
        ui_->data_actuation_command_label_value_motor_right_forward->setText("False");
    }

    ui_->data_actuation_command_label_value_motor_left_pwm->setText(QString::number(message.actuation_command.motor_left_pwm, 'f', 5));
    ui_->data_actuation_command_label_value_motor_right_pwm->setText(QString::number(message.actuation_command.motor_right_pwm, 'f', 5));

    ui_->data_controller_reference_label_value_attitude_y->setText(QString::number(radiansToDegrees(message.controller_reference.attitude_y), 'f', 5));
    ui_->data_controller_reference_label_value_attitude_z->setText(QString::number(radiansToDegrees(message.controller_reference.attitude_z), 'f', 5));
    ui_->data_controller_reference_label_value_position_x->setText(QString::number(message.controller_reference.position_x, 'f', 5));

    ui_->data_imu_data_label_value_acceleration_x->setText(QString::number(message.imu_data.acceleration_x, 'f', 5));
    ui_->data_imu_data_label_value_acceleration_y->setText(QString::number(message.imu_data.acceleration_y, 'f', 5));
    ui_->data_imu_data_label_value_acceleration_z->setText(QString::number(message.imu_data.acceleration_z, 'f', 5));
    ui_->data_imu_data_label_value_attitude_x->setText(QString::number(radiansToDegrees(message.imu_data.attitude_x), 'f', 5));
    ui_->data_imu_data_label_value_attitude_y->setText(QString::number(radiansToDegrees(message.imu_data.attitude_y), 'f', 5));
    ui_->data_imu_data_label_value_attitude_z->setText(QString::number(radiansToDegrees(message.imu_data.attitude_z), 'f', 5));
    ui_->data_imu_data_label_value_angular_velocity_x->setText(QString::number(radiansToDegrees(message.imu_data.angular_velocity_x), 'f', 5));
    ui_->data_imu_data_label_value_angular_velocity_y->setText(QString::number(radiansToDegrees(message.imu_data.angular_velocity_y), 'f', 5));
    ui_->data_imu_data_label_value_angular_velocity_z->setText(QString::number(radiansToDegrees(message.imu_data.angular_velocity_z), 'f', 5));
    ui_->data_imu_data_label_value_compass_x->setText(QString::number(message.imu_data.compass_x, 'f', 5));
    ui_->data_imu_data_label_value_compass_y->setText(QString::number(message.imu_data.compass_y, 'f', 5));
    ui_->data_imu_data_label_value_compass_z->setText(QString::number(message.imu_data.compass_z, 'f', 5));
    ui_->data_imu_data_label_value_temperature->setText(QString::number(message.imu_data.temperature));

    ui_->data_encoder_data_label_value_position_x->setText(QString::number(message.encoder_data.position_x, 'f', 5));
    ui_->data_encoder_data_label_value_steps->setText(QString::number(message.encoder_data.steps));
    ui_->data_encoder_data_label_value_steps_left->setText(QString::number(message.encoder_data.steps_left));
    ui_->data_encoder_data_label_value_steps_right->setText(QString::number(message.encoder_data.steps_right));
    ui_->data_encoder_data_label_value_velocity_x->setText(QString::number(message.encoder_data.velocity_x, 'f', 5));

    if (message.time_of_flight_data.range_left < 10000)
    {
        ui_->data_time_of_flight_data_label_value_range_left->setText(QString::number(message.time_of_flight_data.range_left, 'f', 5));
    }
    else
    {
        ui_->data_time_of_flight_data_label_value_range_left->setText(QString::number(message.time_of_flight_data.range_left, 'g', 3));
    }

    if (message.time_of_flight_data.range_middle < 10000)
    {
        ui_->data_time_of_flight_data_label_value_range_middle->setText(QString::number(message.time_of_flight_data.range_middle, 'f', 5));
    }
    else
    {
        ui_->data_time_of_flight_data_label_value_range_middle->setText(QString::number(message.time_of_flight_data.range_middle, 'g', 3));
    }

    if (message.time_of_flight_data.range_right < 10000)
    {
        ui_->data_time_of_flight_data_label_value_range_right->setText(QString::number(message.time_of_flight_data.range_right, 'f', 5));
    }
    else
    {
        ui_->data_time_of_flight_data_label_value_range_right->setText(QString::number(message.time_of_flight_data.range_right, 'g', 3));
    }

    ui_->controller_parameter_balance_label_value_proportional->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_y.proportional, 'f', 1));
    ui_->controller_parameter_balance_label_value_integral->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_y.integral, 'f', 1));
    ui_->controller_parameter_balance_label_value_differential->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_y.differential, 'f', 1));
    ui_->controller_parameter_balance_label_value_integral_max->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_y.integral_max, 'f', 1));

    ui_->controller_parameter_forward_label_value_proportional->setText(QString::number(message.controller_parameter.pid_controller_gain_position_x.proportional, 'f', 1));
    ui_->controller_parameter_forward_label_value_integral->setText(QString::number(message.controller_parameter.pid_controller_gain_position_x.integral, 'f', 1));
    ui_->controller_parameter_forward_label_value_differential->setText(QString::number(message.controller_parameter.pid_controller_gain_position_x.differential, 'f', 1));
    ui_->controller_parameter_forward_label_value_integral_max->setText(QString::number(message.controller_parameter.pid_controller_gain_position_x.integral_max, 'f', 1));
    ui_->controller_parameter_forward_label_value_input_upper->setText(QString::number(message.controller_parameter.pid_controller_saturation_position_x.input_upper, 'f', 1));
    ui_->controller_parameter_forward_label_value_input_lower->setText(QString::number(message.controller_parameter.pid_controller_saturation_position_x.input_lower, 'f', 1));

    ui_->controller_parameter_turning_label_value_proportional->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_z.proportional, 'f', 1));
    ui_->controller_parameter_turning_label_value_integral->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_z.integral, 'f', 1));
    ui_->controller_parameter_turning_label_value_differential->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_z.differential, 'f', 1));
    ui_->controller_parameter_turning_label_value_integral_max->setText(QString::number(message.controller_parameter.pid_controller_gain_attitude_z.integral_max, 'f', 1));
    ui_->controller_parameter_turning_label_value_open_loop->setText(QString::number(message.controller_parameter.attitude_z_gain_open_loop, 'f', 1));

    ui_->planner_parameter_label_value_forward->setText(QString::number(message.controller_reference.position_x, 'f', 1));
    ui_->planner_parameter_label_value_turning->setText(QString::number(radiansToDegrees(message.controller_reference.attitude_z), 'f', 1));

    if (!controller_parameter_initialized_)
    {
        ui_->controller_parameter_balance_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_attitude_y.proportional);
        ui_->controller_parameter_balance_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_attitude_y.integral);
        ui_->controller_parameter_balance_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_attitude_y.differential);
        ui_->controller_parameter_balance_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_attitude_y.integral_max);

        ui_->controller_parameter_forward_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_position_x.proportional);
        ui_->controller_parameter_forward_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_position_x.integral);
        ui_->controller_parameter_forward_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_position_x.differential);
        ui_->controller_parameter_forward_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_position_x.integral_max);
        ui_->controller_parameter_forward_double_spin_box_input_upper->setValue(message.controller_parameter.pid_controller_saturation_position_x.input_upper);
        ui_->controller_parameter_forward_double_spin_box_input_lower->setValue(message.controller_parameter.pid_controller_saturation_position_x.input_lower);

        ui_->controller_parameter_turning_double_spin_box_proportional->setValue(message.controller_parameter.pid_controller_gain_attitude_z.proportional);
        ui_->controller_parameter_turning_double_spin_box_integral->setValue(message.controller_parameter.pid_controller_gain_attitude_z.integral);
        ui_->controller_parameter_turning_double_spin_box_differential->setValue(message.controller_parameter.pid_controller_gain_attitude_z.differential);
        ui_->controller_parameter_turning_double_spin_box_integral_max->setValue(message.controller_parameter.pid_controller_gain_attitude_z.integral_max);
        ui_->controller_parameter_turning_double_spin_box_open_loop->setValue(message.controller_parameter.attitude_z_gain_open_loop);

        ui_->controller_parameter_balance_label_value_proportional->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_balance_label_value_integral->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_balance_label_value_differential->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_balance_label_value_integral_max->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);

        ui_->controller_parameter_forward_label_value_proportional->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_forward_label_value_integral->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_forward_label_value_differential->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_forward_label_value_integral_max->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_forward_label_value_input_upper->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_forward_label_value_input_lower->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);

        ui_->controller_parameter_turning_label_value_proportional->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_turning_label_value_integral->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_turning_label_value_differential->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_turning_label_value_integral_max->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->controller_parameter_turning_label_value_open_loop->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);

        controller_parameter_initialized_ = true;
        controller_parameter_updated = true;
    }
    else
    {
        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_y.proportional != message.controller_parameter.pid_controller_gain_attitude_y.proportional)
        {
            ui_->controller_parameter_balance_label_value_proportional->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_y.integral != message.controller_parameter.pid_controller_gain_attitude_y.integral)
        {
            ui_->controller_parameter_balance_label_value_integral->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_y.differential != message.controller_parameter.pid_controller_gain_attitude_y.differential)
        {
            ui_->controller_parameter_balance_label_value_differential->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_y.integral_max != message.controller_parameter.pid_controller_gain_attitude_y.integral_max)
        {
            ui_->controller_parameter_balance_label_value_integral_max->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_position_x.proportional != message.controller_parameter.pid_controller_gain_position_x.proportional)
        {
            ui_->controller_parameter_forward_label_value_proportional->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_position_x.integral != message.controller_parameter.pid_controller_gain_position_x.integral)
        {
            ui_->controller_parameter_forward_label_value_integral->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_position_x.differential != message.controller_parameter.pid_controller_gain_position_x.differential)
        {
            ui_->controller_parameter_forward_label_value_differential->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_position_x.integral_max != message.controller_parameter.pid_controller_gain_position_x.integral_max)
        {
            ui_->controller_parameter_forward_label_value_integral_max->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_saturation_position_x.input_upper != message.controller_parameter.pid_controller_saturation_position_x.input_upper)
        {
            ui_->controller_parameter_forward_label_value_input_upper->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_saturation_position_x.input_lower != message.controller_parameter.pid_controller_saturation_position_x.input_lower)
        {
            ui_->controller_parameter_forward_label_value_input_lower->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_z.proportional != message.controller_parameter.pid_controller_gain_attitude_z.proportional)
        {
            ui_->controller_parameter_turning_label_value_proportional->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_z.integral != message.controller_parameter.pid_controller_gain_attitude_z.integral)
        {
            ui_->controller_parameter_turning_label_value_integral->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_z.differential != message.controller_parameter.pid_controller_gain_attitude_z.differential)
        {
            ui_->controller_parameter_turning_label_value_differential->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.pid_controller_gain_attitude_z.integral_max != message.controller_parameter.pid_controller_gain_attitude_z.integral_max)
        {
            ui_->controller_parameter_turning_label_value_integral_max->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }

        if (biped_message_render_.controller_parameter.attitude_z_gain_open_loop != message.controller_parameter.attitude_z_gain_open_loop)
        {
            ui_->controller_parameter_turning_label_value_open_loop->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            controller_parameter_updated = true;
        }
    }

    if (!planner_parameter_initialized_)
    {
        ui_->planner_parameter_double_spin_box_forward->setValue(message.controller_reference.position_x);
        ui_->planner_parameter_double_spin_box_turning->setValue(radiansToDegrees(message.controller_reference.attitude_z));

        ui_->planner_parameter_label_value_forward->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
        ui_->planner_parameter_label_value_turning->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);

        planner_parameter_initialized_ = true;
        planner_parameter_updated = true;
    }
    else
    {
        if (biped_message_render_.controller_reference.position_x != message.controller_reference.position_x)
        {
            ui_->planner_parameter_label_value_forward->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            planner_parameter_updated = true;
        }

        if (biped_message_render_.controller_reference.attitude_z != message.controller_reference.attitude_z)
        {
            ui_->planner_parameter_label_value_turning->setStyleSheet(UIParameter::window_parameter_label_current_style_updated);
            planner_parameter_updated = true;
        }
    }

    ui_->controller_response_plot_balance->addDataPointToCurve(0, duration_since_epoch, radiansToDegrees(message.imu_data.attitude_y));
    ui_->controller_response_plot_balance->addDataPointToCurve(1, duration_since_epoch, radiansToDegrees(message.controller_reference.attitude_y));
    ui_->controller_response_plot_forward->addDataPointToCurve(0, duration_since_epoch, message.encoder_data.position_x);
    ui_->controller_response_plot_forward->addDataPointToCurve(1, duration_since_epoch, message.controller_reference.position_x);
    ui_->controller_response_plot_turning->addDataPointToCurve(0, duration_since_epoch, radiansToDegrees(message.imu_data.attitude_z));
    ui_->controller_response_plot_turning->addDataPointToCurve(1, duration_since_epoch, radiansToDegrees(message.controller_reference.attitude_z));

    if (controller_parameter_updated)
    {
        if (controller_parameter_initialized)
        {
            logToStatusBar("Received updated controller parameters from Biped at \"" + ip_biped_ + "\".");
        }

        QTimer::singleShot(UIParameter::window_parameter_label_current_style_reset_delay, this, &Window::resetControllerParameterLabelCurrentStyle);
    }

    if (planner_parameter_updated)
    {
        if (planner_parameter_initialized)
        {
            logToStatusBar("Received updated planner parameters from Biped at \"" + ip_biped_ + "\".");
        }

        QTimer::singleShot(UIParameter::window_parameter_label_current_style_reset_delay, this, &Window::resetPlannerParameterLabelCurrentStyle);
    }

    biped_message_render_ = message;
    time_point_last_render_biped_message_ = std::chrono::system_clock::now();
}

void
Window::resetControllerParameterLabelCurrentStyle()
{
    ui_->controller_parameter_balance_label_value_proportional->setStyleSheet("");
    ui_->controller_parameter_balance_label_value_integral->setStyleSheet("");
    ui_->controller_parameter_balance_label_value_differential->setStyleSheet("");
    ui_->controller_parameter_balance_label_value_integral_max->setStyleSheet("");

    ui_->controller_parameter_forward_label_value_proportional->setStyleSheet("");
    ui_->controller_parameter_forward_label_value_integral->setStyleSheet("");
    ui_->controller_parameter_forward_label_value_differential->setStyleSheet("");
    ui_->controller_parameter_forward_label_value_integral_max->setStyleSheet("");
    ui_->controller_parameter_forward_label_value_input_upper->setStyleSheet("");
    ui_->controller_parameter_forward_label_value_input_lower->setStyleSheet("");

    ui_->controller_parameter_turning_label_value_proportional->setStyleSheet("");
    ui_->controller_parameter_turning_label_value_integral->setStyleSheet("");
    ui_->controller_parameter_turning_label_value_differential->setStyleSheet("");
    ui_->controller_parameter_turning_label_value_integral_max->setStyleSheet("");
    ui_->controller_parameter_turning_label_value_open_loop->setStyleSheet("");
}

void
Window::resetPlannerParameterLabelCurrentStyle()
{
    ui_->planner_parameter_label_value_forward->setStyleSheet("");
    ui_->planner_parameter_label_value_turning->setStyleSheet("");
}
}
}
