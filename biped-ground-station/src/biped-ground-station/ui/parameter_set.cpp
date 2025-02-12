#include "common/parameter.h"
#include "ui/parameter_set.h"
#include "ui_parameter_set.h"

namespace biped
{
namespace ground_station
{
ParameterSet::ParameterSet(QWidget *parent) : QWidget(parent), index_(0), pinned_(false)
{
    ui_ = std::make_unique<Ui::ParameterSet>();

    if (!ui_)
    {
        return;
    }

    ui_->setupUi(this);

    connect(ui_->controller_parameter_input_push_button_delete, &QPushButton::clicked, this, &ParameterSet::onControllerParameterInputPushButtonDeleteClicked);
    connect(ui_->controller_parameter_input_push_button_load, &QPushButton::clicked, this, &ParameterSet::onControllerParameterInputPushButtonLoadClicked);
    connect(ui_->controller_parameter_input_push_button_notes, &QPushButton::clicked, this, &ParameterSet::onControllerParameterInputPushButtonNotesClicked);
    connect(ui_->top_bar_push_button_pin, &QPushButton::clicked, this, &ParameterSet::onTopBarPushButtonPinClicked);
}

ParameterSet::~ParameterSet()
{
}

biped::firmware::BipedMessage
ParameterSet::getBipedMessage() const
{
    return biped_message_;
}

size_t
ParameterSet::getIndex() const
{
    return index_;
}

std::string
ParameterSet::getName() const
{
    return ui_->group_box_name->title().toStdString();
}

std::string
ParameterSet::getNotes() const
{
    return notes_;
}

bool
ParameterSet::getPinned() const
{
    return pinned_;
}

void
ParameterSet::setBipedMessage(const biped::firmware::BipedMessage& message)
{
    biped_message_ = message;

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
}

void
ParameterSet::setDateTime(const std::string& date_time)
{
    ui_->top_bar_label_date_time->setText(QString::fromStdString(date_time));
}

void
ParameterSet::setIndex(const size_t& index)
{
    index_ = index;
}

void
ParameterSet::setName(const std::string& name)
{
    ui_->group_box_name->setTitle(QString::fromStdString(name));
}

void
ParameterSet::setNotes(const std::string& notes)
{
    notes_ = notes;
}

void
ParameterSet::onControllerParameterInputPushButtonDeleteClicked()
{
    emit pushButtonDeleteClicked(index_);
}

void
ParameterSet::onControllerParameterInputPushButtonLoadClicked()
{
    emit pushButtonLoadClicked(index_);
}

void
ParameterSet::onControllerParameterInputPushButtonNotesClicked()
{
    emit pushButtonNotesClicked(index_);
}

void
ParameterSet::onTopBarPushButtonPinClicked()
{
    if (pinned_)
    {
        ui_->top_bar_push_button_pin->setStyleSheet("");
        ui_->top_bar_push_button_pin->setText("Pin");

        pinned_ = false;
    }
    else
    {
        ui_->top_bar_push_button_pin->setStyleSheet(UIParameter::parameter_set_push_button_pin_style_pinned);
        ui_->top_bar_push_button_pin->setText("Pinned");

        pinned_ = true;
    }

    emit pushButtonPinClicked(index_);
}
}
}
