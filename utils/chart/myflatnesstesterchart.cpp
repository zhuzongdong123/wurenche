#include "myflatnesstesterchart.h"
#include <QPainter>
#include <QDebug>
#include "appconfigbase.h"

MyFlatnessTesterChart::MyFlatnessTesterChart(QWidget *parent) : QWidget(parent)
{
    m_isChangedY= AppConfigBase::getInstance()->readConfigSettings("Setting","AccelerometerYChanged","0").toInt();
}

void MyFlatnessTesterChart::slt_addValues(double key, double value)
{
    if(m_allVec.size() > 10)
    {
        m_allVec.pop_back();
    }
    m_allVec.push_front(QPointF(key,value));

    //重新计算最大值和最小值
    double minValue = 100000;
    double maxValue = -100000;
    for(auto pos : m_allVec)
    {
        double pValue = pos.y();
        if(pValue > maxValue)
            maxValue = pValue;

        if(pValue < minValue)
            minValue = pValue;
    }
    m_yMin = int(minValue-1);
    m_yMax = int(maxValue+1);

    m_xPointVec.clear();
    for(int index = 0; index < m_allVec.size(); index++)
    {
        double value = m_allVec[index].y();

        int yheight = this->height() -m_space*2;
        double y_ave = double(yheight)/(m_yMax-m_yMin);
        double xValue = height()-(((value - m_yMin)*y_ave) + m_space);

        //每间隔xx个像素显示一个
        m_xPointVec.push_back(QPointF(width()-m_space-index*xWidth/10,xValue));
    }

    update();
}

void MyFlatnessTesterChart::start()
{
    m_isStart = true;
    update();
}

void MyFlatnessTesterChart::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    // 绘制半透明背景（RGBA颜色中的Alpha通道控制透明度）
    painter.setBrush(QColor(255,  255, 255, 255)); // 白色，透明度40%
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());

    if(!m_isStart)
        return;

    //绘制曲线
    drawSplineSeries();

    //绘制填充矩形
    drawRcet();

    //绘制横轴
    drawXAxis();

    //绘制纵轴
    drawYAxis();

    //绘制图例
    drawlegend();
}

void MyFlatnessTesterChart::resizeEvent(QResizeEvent *event)
{
    xWidth = width()-m_space*2;
}

void MyFlatnessTesterChart::drawSplineSeries()
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);//抗锯齿，反走样
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true );
    painter.setPen(QPen(QColor(255,0,0), 1));
    painter.drawPolyline(m_xPointVec);//drawPolyline
    painter.setPen(QPen(QColor(0,255,0), 1));
    painter.restore();
}

void MyFlatnessTesterChart::drawXAxis()
{
    QPainter painter(this);
    painter.setFont(m_font);
    QFontMetrics fm(m_font);

    float timeDistanceAve = float(float(this->width())- m_space*2)/(m_xNum);
    for(int i = 0; i < m_xNum+1; i++)
    {
        painter.setPen(QPen(m_textColor, 1));
        //x轴的刻度尺
        painter.drawLine(QPointF(m_space + timeDistanceAve*i,height()-m_space), QPointF(m_space + timeDistanceAve*i,height() - m_space + m_lineLenth));

        //绘制虚线
        //if(m_yNum != i)
        {
            QPen pen(QColor(100,100,100));
            pen.setStyle(Qt::SolidLine);
            painter.setPen(QPen(QColor(130,130,130,200), 1));
            painter.drawLine(QPointF(m_space + timeDistanceAve*i,height()-m_space), QPointF(m_space + timeDistanceAve*i,m_space));
        }
    }

    for(int index = 0; index < m_allVec.size(); index++)
    {
        painter.setPen(QPen(m_textColor, 2));
        double key = m_allVec[index].x();
        float textHeight = fm.boundingRect(QString::number(key)).height();//求文字的像素高度
        float textWidth = fm.boundingRect(QString::number(key)).width();//求文字的像素宽度
        painter.drawText(QPointF(m_space + timeDistanceAve*(m_xNum-index)-textWidth/2,height() - m_space + m_lineLenth+textHeight), QString::number(key));//居中显示
        painter.setPen(QPen(m_textColor, 2));
    }

    //x轴横向线
    painter.setPen(QPen(m_textColor, 1));
    painter.drawLine(QPointF(m_space,height()-m_space), QPointF(QPointF(width()-m_space,height()-m_space)));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);//抗锯齿，反走样
}

void MyFlatnessTesterChart::drawYAxis()
{
    QPainter painter(this);
    painter.setPen(QPen(m_textColor, 2));
    painter.setFont(m_font);
    QFontMetrics fm(m_font);

    double yValueAve = (m_yMax-m_yMin)/m_yNum;

    for(int i = 0; i < m_yNum+1; i++)
    {
        //y轴的刻度尺
        painter.setPen(QPen(m_textColor, 1));
        float timeDistanceAve = float(float(this->height())- m_space*2)/(m_yNum);
        painter.drawLine(QPointF(m_space, m_space + timeDistanceAve*i), QPointF(m_space-m_lineLenth, m_space + timeDistanceAve*i));

        //显示y轴的文字
        double currentValue = m_yMin + i*yValueAve;
        if(fabs(currentValue) < 1e-5)
            currentValue = 0;
        QString outPutText = QString::number(currentValue,'f',1);
        if(outPutText.endsWith("0"))
            outPutText.chop(2);
        float textHeight = fm.boundingRect(outPutText).height();//求文字的像素高度
        float textWidth = fm.boundingRect(outPutText).width();//求文字的像素宽度
        painter.drawText(m_space-m_lineLenth-textWidth-4, height()-m_space-timeDistanceAve*i+textHeight/2-1, outPutText);//居中显示

        //绘制虚线
        if(m_yNum != i)
        {
            QPen pen(QColor(100,100,100));
            pen.setStyle(Qt::SolidLine);
            painter.setPen(QPen(QColor(130,130,130,200), 1));
            painter.drawLine(QPointF(m_space, m_space + timeDistanceAve*i), QPointF(width()-m_space, m_space + timeDistanceAve*i));
        }
    }

    //y轴横向线
    painter.setPen(QPen(m_textColor, 1));
    painter.drawLine(QPointF(m_space,height()-m_space), QPointF(m_space,m_space));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);//抗锯齿，反走样
}

void MyFlatnessTesterChart::drawlegend()
{
    QPainter painter(this);
    painter.setFont(m_font);
    QFontMetrics fm(m_font);

    {
        QString outPutText = "平整度仪";
        float textHeight = fm.boundingRect(outPutText).height();//求文字的像素高度
        painter.setPen(QPen(QColor(100,100,100), 2));
        painter.drawText(width()*1/4-50+5, m_space/2+textHeight/2, outPutText);

        painter.setPen(QPen(QColor(255,0,0), 5));
        painter.drawLine(QPointF(width()*1/4-52, m_space/2), QPointF(width()*1/4-70, m_space/2));
    }
}

void MyFlatnessTesterChart::drawRcet()
{
    QPainter painter(this);
    //先绘制个填充矩形
    painter.setPen(QPen(QColor(255,255,255), 2));
    painter.setBrush(QColor(255,255,255));
    painter.drawRect(QRect(QPoint(0,height()-m_space),QPoint(width(),height())));
    painter.drawRect(QRect(QPoint(0,0),QPoint(width(),m_space)));
}
