#include "mychart.h"
#include <QPainter>
#include <QDebug>
#include "appconfigbase.h"
#include <QTimer>

MyChart::MyChart(QWidget *parent) : QWidget(parent)
{
    m_isChangedY= AppConfigBase::getInstance()->readConfigSettings("Setting","AccelerometerYChanged","0").toInt();
}

void MyChart::addValues(double x, double y, double z)
{
    if(!m_isStart)
        m_isStart = true;

    //显示范围超过可视范围，清空部分数据
    if(m_xPointVecTemp.size() == m_xNum*100)
    {
        if(m_xPointVecTemp.size() > 0)
        {
            m_xPointVecTemp.pop_back();
            m_yPointVecTemp.pop_back();
            m_zPointVecTemp.pop_back();
        }

        if(m_allValue.size() > 3)
        {
            m_allValue.pop_back();
            m_allValue.pop_back();
            m_allValue.pop_back();
        }
    }

    //计算y轴的显示范围
    m_allValue.push_front(x);
    m_allValue.push_front(y);
    m_allValue.push_front(z);

    if(m_isChangedY)
    {
        double yMin = -0.2;
        double yMax = 1.2;
        for(auto value : m_allValue)
        {
            if(value > yMax)
                yMax = value;

            if(value < yMin)
                yMin = value;
        }
        m_yMax = yMax;
        m_yMin = yMin;
    }


    //纵轴高度
    int yheight = this->height() -m_space*2;
    double y_ave = double(yheight)/(m_yMax-m_yMin);
    {
        double xValue = height()-(((x - m_yMin)*y_ave) + m_space);
        m_xPointVecTemp.push_front(xValue);
    }
    {
        double yValue = height()-(((y - m_yMin)*y_ave) + m_space);
        m_yPointVecTemp.push_front(yValue);
    }
    {
        double zValue = height()-(((z - m_yMin)*y_ave) + m_space);
        m_zPointVecTemp.push_front(zValue);
    }

    //倒序显示
    m_xPointVec.clear();
    m_yPointVec.clear();
    m_zPointVec.clear();
    int count = m_xPointVecTemp.size();
    for(int index = 0; index < count; index++)
    {
        m_xPointVec.push_back(QPointF(m_space+xWidth-index,m_xPointVecTemp[index]));
        m_yPointVec.push_back(QPointF(m_space+xWidth-index,m_yPointVecTemp[index]));
        m_zPointVec.push_back(QPointF(m_space+xWidth-index,m_zPointVecTemp[index]));
    }

    update();
}

void MyChart::start()
{
    m_isStart = true;
    update();
}

void MyChart::paintEvent(QPaintEvent *event)
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

void MyChart::resizeEvent(QResizeEvent *event)
{
    xWidth = width()-m_space*2;
}

void MyChart::drawSplineSeries()
{
    QPainter painter(this);
    painter.save();
    painter.setPen(Qt::blue);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true );
    painter.setPen(QPen(QColor(255,0,0), 1));
    painter.drawPolyline(m_xPointVec);//drawPolyline
    painter.setPen(QPen(QColor(0,255,0), 1));
    painter.drawPolyline(m_yPointVec);
    painter.setPen(QPen(QColor(0,0,255), 1));
    painter.drawPolyline(m_zPointVec);
    painter.restore();
}

void MyChart::drawXAxis()
{
    QPainter painter(this);
    painter.setFont(m_font);
    QFontMetrics fm(m_font);

    for(int i = 0; i < m_xNum+1; i++)
    {
        painter.setPen(QPen(m_textColor, 1));
        //x轴的刻度尺
        float timeDistanceAve = float(float(this->width())- m_space*2)/(m_xNum);
        painter.drawLine(QPointF(m_space + timeDistanceAve*i,height()-m_space), QPointF(m_space + timeDistanceAve*i,height() - m_space + m_lineLenth));

        //显示x轴的文字
        QString outPutText = QString::number(i*100);
        float textHeight = fm.boundingRect(outPutText).height();//求文字的像素高度
        float textWidth = fm.boundingRect(outPutText).width();//求文字的像素宽度
        painter.drawText(m_space + timeDistanceAve*i-textWidth/2,height() - m_space + m_lineLenth+textHeight, outPutText);//居中显示

        //绘制虚线
        if(m_yNum != i)
        {
            QPen pen(QColor(100,100,100));
            pen.setStyle(Qt::SolidLine);
            painter.setPen(QPen(QColor(130,130,130,200), 1));
            painter.drawLine(QPointF(m_space + timeDistanceAve*i,height()-m_space), QPointF(m_space + timeDistanceAve*i,m_space));
        }
    }

    //x轴横向线
    painter.setPen(QPen(m_textColor, 1));
    painter.drawLine(QPointF(m_space,height()-m_space), QPointF(QPointF(width()-m_space,height()-m_space)));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);//抗锯齿，反走样
}

void MyChart::drawYAxis()
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

void MyChart::drawlegend()
{
    QPainter painter(this);
    painter.setFont(m_font);
    QFontMetrics fm(m_font);
    {
        QString outPutText = "加速度计-X";
        float textHeight = fm.boundingRect(outPutText).height();//求文字的像素高度
        painter.setPen(QPen(QColor(100,100,100), 2));
        painter.drawText(width()*1/4-50+5, m_space/2+textHeight/2, outPutText);

        painter.setPen(QPen(QColor(255,0,0), 5));
        painter.drawLine(QPointF(width()*1/4-52, m_space/2), QPointF(width()*1/4-70, m_space/2));
    }

    {
        QString outPutText = "加速度计-Y";
        float textHeight = fm.boundingRect(outPutText).height();//求文字的像素高度
        painter.setPen(QPen(QColor(100,100,100), 2));
        painter.drawText(width()*2/4-50+5, m_space/2+textHeight/2, outPutText);

        painter.setPen(QPen(QColor(0,255,0), 5));
        painter.drawLine(QPointF(width()*2/4-52, m_space/2), QPointF(width()*2/4-70, m_space/2));
    }

    {
        QString outPutText = "加速度计-Z";
        float textHeight = fm.boundingRect(outPutText).height();//求文字的像素高度
        painter.setPen(QPen(QColor(100,100,100), 2));
        painter.drawText(width()*3/4-50+5, m_space/2+textHeight/2, outPutText);

        painter.setPen(QPen(QColor(0,0,255), 5));
        painter.drawLine(QPointF(width()*3/4-52, m_space/2), QPointF(width()*3/4-70, m_space/2));
    }

}

void MyChart::drawRcet()
{
    QPainter painter(this);
    //先绘制个填充矩形
    painter.setPen(QPen(QColor(255,  255, 255, 255), 2));
    painter.setBrush(QColor(255,  255, 255, 255));
    painter.drawRect(QRect(QPoint(0,height()-m_space),QPoint(width(),height())));
    painter.drawRect(QRect(QPoint(0,0),QPoint(width(),m_space)));
    painter.drawRect(QRect(QPoint(0,0),QPoint(m_space,height())));
}
