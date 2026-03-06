#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QDateTimeAxis>
#include <QSplineSeries>
#include <QValueAxis>
#include <QTimer>
#include "QDateTime"

class ChartWidget : public QtCharts::QChartView
{
    Q_OBJECT
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    void createChart();
    void createSeries(QString objName, QColor color);
    void setYTrickCount(int count);

public slots:
    void slt_drawLine(QMap<QString,double> mapValue);

private:
    QTimer *timer;
    QMap<QString, QtCharts::QSplineSeries*> m_seriesMap;
    QtCharts::QChart *chart = nullptr;                           //画布
    QtCharts::QDateTimeAxis *axisX = nullptr;                    //轴
    QtCharts::QValueAxis *axisY = nullptr;
};

#endif // CHARTWIDGET_H
