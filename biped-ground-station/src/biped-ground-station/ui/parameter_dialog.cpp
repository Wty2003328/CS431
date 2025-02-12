#include "ui/parameter_dialog.h"
#include "common/parameter.h"
#include "ui_parameter_dialog.h"

namespace biped
{
namespace ground_station
{
ParameterDialog::ParameterDialog(QWidget* parent) : QDialog(parent)
{
    ui_ = std::make_unique<Ui::ParameterDialog>();

    if (!ui_)
    {
        return;
    }

    ui_->setupUi(this);

    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowModality(Qt::ApplicationModal);

    layout()->setSizeConstraint(QLayout::SetFixedSize);

    ui_->dialog_button_box->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    ui_->text_edit_notes->setFixedHeight(UIParameter::parameter_dialog_text_edit_height_fixed);
    ui_->text_edit_notes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    connect(ui_->dialog_button_box, &QDialogButtonBox::clicked, this, &ParameterDialog::onDialogButtonBoxClicked);
}

ParameterDialog::~ParameterDialog()
{
}

void
ParameterDialog::addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role)
{
    ui_->dialog_button_box->addButton(button, role);
}

QPushButton*
ParameterDialog::addButton(const QString& text, QDialogButtonBox::ButtonRole role)
{
    QPushButton* button = ui_->dialog_button_box->addButton(text, role);

    if (button)
    {
        button->setAutoDefault(false);
        button->setDefault(false);
        button->setMinimumWidth(UIParameter::confirm_dialog_button_width_minimum);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }

    return button;
}

QPushButton*
ParameterDialog::addButton(QDialogButtonBox::StandardButton button)
{
    QPushButton* button_push = ui_->dialog_button_box->addButton(button);

    if (button_push)
    {
        button_push->setAutoDefault(false);
        button_push->setDefault(false);
        button_push->setMinimumWidth(UIParameter::confirm_dialog_button_width_minimum);
        button_push->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }

    return button_push;
}

void
ParameterDialog::clear()
{
    ui_->dialog_button_box->clear();
    button_clicked_ = nullptr;
    button_default_ = nullptr;
}

QAbstractButton*
ParameterDialog::clickedButton() const
{
    return button_clicked_;
}

QPushButton*
ParameterDialog::defaultButton() const
{
    return button_default_;
}

QString
ParameterDialog::name() const
{
    return ui_->line_edit_name->text();
}

QString
ParameterDialog::notes() const
{
    return ui_->text_edit_notes->toPlainText();
}

void
ParameterDialog::removeButton(QAbstractButton *button)
{
    ui_->dialog_button_box->removeButton(button);
}

void
ParameterDialog::setDefaultButton(QPushButton* button)
{
    button_default_ = button;
    button_clicked_ = button_default_;

    if (button_default_)
    {
        button_default_->setAutoDefault(true);
        button_default_->setDefault(true);
        button_default_->setFocus();
    }
}

void
ParameterDialog::setDefaultButton(QDialogButtonBox::StandardButton button)
{
    button_default_ = ui_->dialog_button_box->button(button);
    button_clicked_ = button_default_;

    if (button_default_)
    {
        button_default_->setAutoDefault(true);
        button_default_->setDefault(true);
        button_default_->setFocus();
    }
}

void
ParameterDialog::setName(const QString& name)
{
    return ui_->line_edit_name->setText(name);
}

void
ParameterDialog::setNotes(const QString& notes)
{
    return ui_->text_edit_notes->setPlainText(notes);
}

void
ParameterDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void
ParameterDialog::onDialogButtonBoxClicked(QAbstractButton* button)
{
    button_clicked_ = button;
    close();
}
}
}
