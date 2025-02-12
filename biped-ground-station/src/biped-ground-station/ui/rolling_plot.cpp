#include "common/parameter.h"
#include "ui/rolling_plot.h"

namespace biped
{
namespace ground_station
{
RollingPlot::RollingPlot(QWidget *parent) : QCustomPlot(parent), rolling_window_capacity_(UIParameter::rolling_plot_rolling_window_capacity_default), y_range_lower_(0), y_range_upper_(0)
{
}

void
RollingPlot::addCurve()
{
    addGraph();
}

void
RollingPlot::addCurves(const size_t& count)
{
    for (size_t i = 0; i < count; i ++)
    {
        addCurve();
    }
}

void
RollingPlot::addCurves(const std::vector<QPen>& pens)
{
    for (size_t i = 0; i < pens.size(); i ++)
    {
        addCurve();
        setCurvePen(i, pens[i]);
    }
}

void
RollingPlot::addDataPointToCurve(const size_t& index, const double& x, const double& y)
{
    graph(index)->addData(x, y);

    updateAxisRangeX(index, x);
    updateAxisRangeY(y);

    replot();
}

void
RollingPlot::removeCurve(const size_t& index)
{
    removeGraph(index);
}

void
RollingPlot::setCurvePen(const size_t& index, const QPen& pen)
{
    graph(index)->setPen(pen);
}

void
RollingPlot::setRollingWindowCapacity(const unsigned& rolling_window_capacity)
{
    rolling_window_capacity_ = rolling_window_capacity;
}

void
RollingPlot::zoomToFit()
{
    double y_range_lower = std::numeric_limits<double>::max();
    double y_range_upper = std::numeric_limits<double>::lowest();

    for (int index_graph = 0; index_graph < graphCount(); index_graph ++)
    {
        for (int index_data = 0; index_data < graph(index_graph)->data()->size(); index_data ++)
        {
            const double data = graph(index_graph)->data()->at(index_data)->value;

            y_range_lower = data < y_range_lower ? data : y_range_lower;
            y_range_upper = data > y_range_upper ? data : y_range_upper;
        }
    }

    y_range_lower_ = y_range_lower;
    y_range_upper_ = y_range_upper;

    yAxis->setRange(y_range_lower_, y_range_upper_);

    replot();
}

void
RollingPlot::updateAxisRangeX(const size_t& index_curve, const double& x)
{
    int index_data_removal = graph(index_curve)->dataCount() - rolling_window_capacity_;
    double x_range_lower = 0;

    if (index_data_removal > 0)
    {
        x_range_lower = graph(index_curve)->data()->at(index_data_removal)->key;
        graph(index_curve)->data()->removeBefore(x_range_lower);
    }
    else if (graph(index_curve)->dataCount() > 0)
    {
        x_range_lower = graph(index_curve)->data()->at(0)->key;
    }

    xAxis->setRange(x_range_lower, x);
}

void
RollingPlot::updateAxisRangeY(const double& y)
{
    y_range_upper_ = y > y_range_upper_ ? y : y_range_upper_;
    y_range_lower_ = y < y_range_lower_ ? y : y_range_lower_;

    yAxis->setRange(y_range_lower_, y_range_upper_);
}
}
}
