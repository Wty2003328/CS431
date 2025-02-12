#include "ui/confirm_dialog.h"
#include "common/parameter.h"
#include "ui_confirm_dialog.h"

namespace biped
{
namespace ground_station
{
ConfirmDialog::ConfirmDialog(QWidget* parent) : QDialog(parent)
{
    ui_ = std::make_unique<Ui::ConfirmDialog>();

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

    QFont font_text_label = ui_->text_label->font();
    font_text_label.setBold(true);
    font_text_label.setPointSize(font_text_label.pointSize() * UIParameter::confirm_dialog_text_resize_factor);
    ui_->text_label->setFont(font_text_label);

    QFont font_text_informative_label = ui_->text_informative_label->font();
    font_text_informative_label.setPointSize(font_text_informative_label.pointSize() * UIParameter::confirm_dialog_text_informative_resize_factor);
    ui_->text_informative_label->setFont(font_text_informative_label);

    connect(ui_->dialog_button_box, &QDialogButtonBox::clicked, this, &ConfirmDialog::onDialogButtonBoxClicked);
}

ConfirmDialog::~ConfirmDialog()
{
}

void
ConfirmDialog::addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role)
{
    ui_->dialog_button_box->addButton(button, role);
}

QPushButton*
ConfirmDialog::addButton(const QString& text, QDialogButtonBox::ButtonRole role)
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
ConfirmDialog::addButton(QDialogButtonBox::StandardButton button)
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
ConfirmDialog::clear()
{
    ui_->dialog_button_box->clear();
    button_clicked_ = nullptr;
    button_default_ = nullptr;
}

QAbstractButton*
ConfirmDialog::clickedButton() const
{
    return button_clicked_;
}

QPushButton*
ConfirmDialog::defaultButton() const
{
    return button_default_;
}

QString
ConfirmDialog::informativeText() const
{
    return ui_->text_informative_label->text();
}

void
ConfirmDialog::removeButton(QAbstractButton *button)
{
    ui_->dialog_button_box->removeButton(button);
}

void
ConfirmDialog::setDefaultButton(QPushButton* button)
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
ConfirmDialog::setDefaultButton(QDialogButtonBox::StandardButton button)
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
ConfirmDialog::setInformativeText(const QString& text)
{
    ui_->text_informative_label->setText(text);
}

void
ConfirmDialog::setText(const QString& text)
{
    ui_->text_label->setText(text);
}

void
ConfirmDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
}

QString
ConfirmDialog::text() const
{
    return ui_->text_label->text();
}

void
ConfirmDialog::onDialogButtonBoxClicked(QAbstractButton* button)
{
    button_clicked_ = button;
    close();
}
}
}
