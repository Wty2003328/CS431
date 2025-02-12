#ifndef WINDOW_H
#define WINDOW_H

#include <chrono>
#include <memory>
#include <mutex>
#include <QGraphicsScene>
#include <QWidget>

#include "common/type.h"

class QLabel;
class QThread;

namespace Ui
{
class Window;
}

namespace biped
{
namespace ground_station
{
class ParameterSet;
class UDP;

class Window : public QWidget
{
    Q_OBJECT

public:

    explicit Window(QWidget *parent = nullptr);

    ~Window();

    void
    logToStatusBar(const std::string& status);

    void
    setRenderingFPSCapBipedMessage(const unsigned& rendering_fps_cap_biped_message);

    void
    setRenderingFPSCapCameraFrame(const unsigned& rendering_fps_cap_camera_frame);

signals:

    void
    operateCameraDaemon();

    void
    operateInboundDaemon();

    void
    operateJoypadDaemon();

    void
    operateOutboundDaemon(const biped::firmware::BipedMessage& message);

private slots:

    void
    onCameraDaemonFrameReceived(const QImage& frame);

    void
    onControllerInputPushButtonApplyClicked();

    void
    onControllerInputPushButtonRevertClicked();

    void
    onControllerInputPushButtonSaveClicked();

    void
    onControllerResponsePlotPushButtonZoomToFitBalanceClicked();

    void
    onControllerResponsePlotPushButtonZoomToFitForwardClicked();

    void
    onControllerResponsePlotPushButtonZoomToFitTurningClicked();

    void
    onInboundDaemonMessageReceived(const biped::firmware::BipedMessage& message);

    void
    onParametersInputPushButtonDeleteAllClicked();

    void
    onParameterSetPushButtonDeleteClicked(const size_t& parameter_set_index);

    void
    onParameterSetPushButtonLoadClicked(const size_t& parameter_set_index);

    void
    onParameterSetPushButtonNotesClicked(const size_t& parameter_set_index);

    void
    onParameterSetPushButtonPinClicked(const size_t& parameter_set_index);

    void
    onPlannerJoyPadXChanged(const double& value);

    void
    onPlannerJoyPadYChanged(const double& value);

    void
    onPlannerParameterPushButtonApplyClicked();

    void
    onPlannerParameterPushButtonRevertClicked();

    void
    onSettingsBipedIPAddressLineEditEditingFinished();

    void
    onSettingsBipedIPAddressPushButtonApplyClicked();

    void
    onSettingsLoggingPushButtonStartClicked();

    void
    onSettingsLoggingPushButtonStopClicked();

    void
    onSettingsThemePushButtonLoadClicked();

    void
    onSettingsThemePushButtonResetClicked();

private:

    void
    connectSignals();

    void
    disconnectSignals();

    void
    initialize();

    void
    renderCameraFrame(const QImage& frame);

    void
    renderBipedMessage(const biped::firmware::BipedMessage& message);

    void
    resetControllerParameterLabelCurrentStyle();

    void
    resetPlannerParameterLabelCurrentStyle();

    biped::firmware::BipedMessage biped_message_;
    biped::firmware::BipedMessage biped_message_render_;
    bool controller_parameter_initialized_;
    std::mutex mutex_biped_message_;
    std::mutex mutex_status_bar_;
    size_t parameter_set_pinned_count_;
    std::vector<ParameterSet*> parameter_sets_;
    bool planner_parameter_initialized_;
    unsigned rendering_fps_cap_biped_message_;
    unsigned rendering_fps_cap_camera_frame_;
    std::unique_ptr<QThread> thread_daemon_camera_;
    std::unique_ptr<QThread> thread_daemon_inbound_;
    std::unique_ptr<QThread> thread_daemon_joypad_;
    std::unique_ptr<QThread> thread_daemon_logging_;
    std::unique_ptr<QThread> thread_daemon_outbound_;
    std::chrono::time_point<std::chrono::system_clock> time_point_last_render_biped_message_;
    std::chrono::time_point<std::chrono::system_clock> time_point_last_render_camera_frame_;
    std::unique_ptr<Ui::Window> ui_;
};
}
}

#endif // WINDOW_H
