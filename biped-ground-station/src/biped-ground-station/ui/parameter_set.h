#ifndef PARAMETER_SET_H
#define PARAMETER_SET_H

#include <memory>
#include <QWidget>

#include "common/type.h"

namespace Ui
{
class ParameterSet;
}

namespace biped
{
namespace ground_station
{
class ParameterSet : public QWidget
{
    Q_OBJECT

public:

    explicit ParameterSet(QWidget *parent = nullptr);

    ~ParameterSet();

    biped::firmware::BipedMessage
    getBipedMessage() const;

    size_t
    getIndex() const;

    std::string
    getName() const;

    std::string
    getNotes() const;

    bool
    getPinned() const;

    void
    setBipedMessage(const biped::firmware::BipedMessage& message);

    void
    setDateTime(const std::string& date_time);

    void
    setIndex(const size_t& index);

    void
    setName(const std::string& name);

    void
    setNotes(const std::string& notes);

signals:

    void
    pushButtonDeleteClicked(const size_t& parameter_set_index);

    void
    pushButtonLoadClicked(const size_t& parameter_set_index);

    void
    pushButtonNotesClicked(const size_t& parameter_set_index);

    void
    pushButtonPinClicked(const size_t& parameter_set_index);

private slots:

    void
    onControllerParameterInputPushButtonDeleteClicked();

    void
    onControllerParameterInputPushButtonLoadClicked();

    void
    onControllerParameterInputPushButtonNotesClicked();

    void
    onTopBarPushButtonPinClicked();

private:

    biped::firmware::BipedMessage biped_message_;
    bool pinned_;
    size_t index_;
    std::string notes_;
    std::unique_ptr<Ui::ParameterSet> ui_;
};
}
}

#endif // PARAMETER_SET_H
