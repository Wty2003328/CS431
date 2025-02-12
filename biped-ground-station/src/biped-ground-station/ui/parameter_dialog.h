#ifndef PARAMETER_DIALOG_H
#define PARAMETER_DIALOG_H

#include <memory>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Ui
{
class ParameterDialog;
}

namespace biped
{
namespace ground_station
{
class ParameterDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ParameterDialog(QWidget* parent = nullptr);

    ~ParameterDialog();

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
    name() const;

    QString
    notes() const;

    void
    removeButton(QAbstractButton* button);

    void
    setDefaultButton(QPushButton* button);

    void
    setDefaultButton(QDialogButtonBox::StandardButton button);

    void
    setName(const QString& name);

    void
    setNotes(const QString& notes);

    void
    setTitle(const QString& title);

private slots:

    void
    onDialogButtonBoxClicked(QAbstractButton* button);

private:

    QAbstractButton* button_clicked_;
    QPushButton* button_default_;
    std::unique_ptr<Ui::ParameterDialog> ui_;
};
}
}

#endif // PARAMETER_DIALOG_H
