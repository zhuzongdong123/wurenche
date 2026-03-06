#ifndef MYCHART_H
#define MYCHART_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class MyChart : public QWidget
{
    Q_OBJECT
public:
    explicit MyChart(QWidget *parent = nullptr);

    void addValues(double x, double y, double z);

    void start();
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:

private:
    void drawSplineSeries();
    void drawXAxis();
    void drawYAxis();
    void drawlegend();
    void drawRcet();//绘制忝填充矩形，防止曲线波动到外边

private:
    QVector<QPointF> m_xPointVec;
    QVector<QPointF> m_yPointVec;
    QVector<QPointF> m_zPointVec;

    QVector<double> m_xPointVecTemp;
    QVector<double> m_yPointVecTemp;
    QVector<double> m_zPointVecTemp;

    QVector<double> m_allValue;

    int m_space = 50;//上下左右间距
    int m_xNum = 6;//横轴显示个数
    int m_yNum = 7;//纵轴显示个数
    int m_lineLenth = 5;//刻度尺长度
    QFont m_font;
    QColor m_textColor = QColor(0,0,0);

    double m_yMin = -0.2;
    double m_yMax = 1.2;

    bool m_isStart = false;
    bool m_isChangedY = false;

    int xWidth = 600;
};

#endif // MYCHART_H
