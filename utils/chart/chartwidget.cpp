#include "chartwidget.h"
#include <QDebug>
#include <QApplication>
ChartWidget::ChartWidget(QWidget *parent) : QtCharts::QChartView(parent)
{
    chart = new QtCharts::QChart();
    chart->setMargins(QMargins(0, 0, 0, 0));//设置内边界全部为0
    chart->setBackgroundBrush(QBrush(QColor(0,0,0)));
    setChart(chart);
}

void ChartWidget::slt_drawLine(QMap<QString,double> mapValue)
{
    if(nullptr == chart || nullptr == axisX || nullptr == axisY)
    {
        return;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    //设置坐标轴显示范围
    chart->axisX()->setMin(QDateTime::currentDateTime().addSecs(-60 * 1));       //系统当前时间的前一秒
    chart->axisX()->setMax(QDateTime::currentDateTime().addSecs(0));            //系统当前时间

    QMap<QString,double>::iterator ite;
    for(auto ite = mapValue.begin(); ite != mapValue.end(); ite++)
    {
        QString objName = ite.key();
        double value = ite.value();
        if(m_seriesMap.find(objName) != m_seriesMap.end())
        {
            QtCharts::QSplineSeries* series = m_seriesMap.find(objName).value();
            int count = series->count();
            if(objName == "AccelerometerSpeed" && count > (20*value))
            {
                series->removePoints(0,1);
            }

            //增加新的点到曲线末端
            series->append(currentTime.toMSecsSinceEpoch(), value);
            //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.ddd") << objName << "当前数据(rcv): " << value << count;
        }

        if(objName == "AccelerometerSpeed" && value > 1)
        {
            chart->axisX()->setMin(QDateTime::currentDateTime().addSecs(-value * 1));       //系统当前时间的前一秒
            chart->axisX()->setMax(QDateTime::currentDateTime().addSecs(0));            //系统当前时间
        }
    }

    //计算当前区间内所有的最大值和最小值
    double minValue = 1000000;//纵轴的最小值
    double maxValue = -1000000;
    for(auto series : m_seriesMap)
    {
        QList<QPointF>points = series->points();
        for(auto p : points)
        {
            //计算纵轴的最大值和最小值
            if(p.ry() > maxValue)
                maxValue = p.ry();
            if(p.ry() < minValue)
                minValue = p.ry();
        }
    }

    //qDebug() << "最小值" << minValue << "最大值" << maxValue;
    chart->axisY()->setMin(minValue-0.1);
    chart->axisY()->setMax(maxValue+0.1);

    QApplication::processEvents();
}

void ChartWidget::createChart()
{
    if(nullptr == axisX)
    {
        axisX = new QtCharts::QDateTimeAxis();
    }

    if(nullptr == axisY)
    {
        axisY = new QtCharts::QValueAxis();
    }

    QPen penY(QColor(255,255,255),2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    for(auto series : m_seriesMap)
    {
        chart->addSeries(series);                            //把线添加到chart
    }
    axisX->setTickCount(10);                             //设置坐标轴格数
    axisY->setTickCount(6);
    axisX->setFormat("mm:ss");                        //设置时间显示格式
    axisY->setMin(0);                                    //设置Y轴范围
    axisY->setMax(10);
    axisX->setTitleText("实时时间");                       //设置X轴名称
    axisX->setTitleVisible(false);
    axisY->setLinePenColor(QColor(255,255,255));        //设置坐标轴颜色样式
    axisY->setGridLineColor(QColor(255,255,255));

    axisY->setGridLineVisible(true);                    //设置Y轴网格不显示
    axisY->setLinePen(penY);
    axisX->setLinePen(penY);

    axisY->setLabelsBrush(QBrush(QColor(255,255,255)));
    axisX->setLabelsBrush(QBrush(QColor(255,255,255)));

//    axisY->setGridLinePen(QColor(255,255,255));
//    axisY->setGridLinePen(QColor(255,255,255));

    QPen gridLinePen(Qt::DashLine);
    gridLinePen.setColor(QColor(130,130,130));

    axisY->setGridLinePen(gridLinePen);
    axisX->setGridLinePen(gridLinePen);//网格的颜色
    axisX->setMin(QDateTime::currentDateTime().addSecs(-60 * 1));       //系统当前时间的前一秒
    axisX->setMax(QDateTime::currentDateTime().addSecs(0));            //系统当前时间

    //series->setPen(gridLinePen);

    chart->addAxis(axisX,Qt::AlignBottom);               //设置坐标轴位于chart中的位置
    chart->addAxis(axisY,Qt::AlignLeft);

    for(auto series : m_seriesMap)
    {
        series->attachAxis(axisX);                           //把数据添加到坐标轴上
        series->attachAxis(axisY);
    }

    axisY->setTitleText("y1");
    axisY->setTitleVisible(false);

    //把chart显示到窗口上
    setContentsMargins(0,0,0,0);
    setRenderHint(QPainter::Antialiasing);   //设置抗锯齿
}

void ChartWidget::createSeries(QString objName, QColor color)
{
    if(m_seriesMap.find(objName) == m_seriesMap.end())
    {
        QtCharts::QSplineSeries* series = new QtCharts::QSplineSeries;
        series->setPen(color);
        series->setName(objName);
        m_seriesMap.insert(objName,series);
    }
}

void ChartWidget::setYTrickCount(int count)
{
    if(nullptr != axisY)
    {
        axisY->setTickCount(count);
    }
}
