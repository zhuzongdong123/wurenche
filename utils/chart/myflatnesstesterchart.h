#ifndef MYFlatnessTesterCHART_H
#define MYFlatnessTesterCHART_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class MyFlatnessTesterChart : public QWidget
{
    Q_OBJECT
public:
    explicit MyFlatnessTesterChart(QWidget *parent = nullptr);

    void start();

public slots:
    void slt_addValues(double key, double value);

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
    QVector<QPointF> m_allVec;
    int m_space = 50;//上下左右间距
    int m_xNum = 10;//横轴显示个数
    int m_yNum = 7;//纵轴显示个数
    int m_lineLenth = 5;//刻度尺长度
    QFont m_font;
    QColor m_textColor = QColor(0,0,0);

    double m_yMin = 0;
    double m_yMax = 10;

    bool m_isStart = false;
    bool m_isChangedY = false;
    int xWidth = 600;
};

#endif // MYCHART_H
