#ifndef ROLLING_PLOT_H
#define ROLLING_PLOT_H

#include "ui/qcustomplot.h"

namespace biped
{
namespace ground_station
{
class RollingPlot : public QCustomPlot
{
    Q_OBJECT

public:

    explicit RollingPlot(QWidget *parent = nullptr);

    void
    addCurve();

    void
    addCurves(const size_t& count);

    void
    addCurves(const std::vector<QPen>& pens);

    void
    addDataPointToCurve(const size_t& index, const double& x, const double& y);

    void
    removeCurve(const size_t& index);

    void
    setCurvePen(const size_t& index, const QPen& pen);

    void
    setRollingWindowCapacity(const unsigned& rolling_window_capacity);

    void
    zoomToFit();

private:

    void
    updateAxisRangeX(const size_t& index_curve, const double& y);

    void
    updateAxisRangeY(const double& y);

    unsigned rolling_window_capacity_;
    double y_range_lower_;
    double y_range_upper_;
};
}
}

#endif // ROLLING_PLOT_H
