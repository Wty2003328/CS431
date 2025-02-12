#ifndef CONFIRM_DIALOG_H
#define CONFIRM_DIALOG_H

#include <memory>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Ui
{
class ConfirmDialog;
}

namespace biped
{
namespace ground_station
{
class ConfirmDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ConfirmDialog(QWidget* parent = nullptr);

    ~ConfirmDialog();

    void
    addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role);

    QPushButton*
    addButton(const QString& text, QDialogButtonBox::ButtonRole role);

    QPushButton*
    addButton(QDialogButtonBox::StandardButton button);

    void
    clear();

    QAbstractButton*
    clickedButton() const;

    QPushButton*
    defaultButton() const;

    QString
    informativeText() const;

    void
    removeButton(QAbstractButton* button);

    void
    setDefaultButton(QPushButton* button);

    void
    setDefaultButton(QDialogButtonBox::StandardButton button);

    void
    setInformativeText(const QString& text);

    void
    setText(const QString& text);

    void
    setTitle(const QString& title);

    QString
    text() const;

private slots:

    void
    onDialogButtonBoxClicked(QAbstractButton* button);

private:

    QAbstractButton* button_clicked_;
    QPushButton* button_default_;
    std::unique_ptr<Ui::ConfirmDialog> ui_;
};
}
}

#endif // CONFIRM_DIALOG_H
