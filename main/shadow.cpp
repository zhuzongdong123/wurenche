#include "shadow.h"
#include <QPainter>

shadow::shadow(QWidget *parent) : QWidget(parent)
{

}

void shadow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制阴影
    painter.setBrush(QColor(255,  255, 255));  // 阴影颜色
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRect(10,  10, width()-10, height()-10), 10, 10);  // 阴影位置和大小

    // 绘制背景
    painter.setBrush(QColor(255,  0, 0, 180));  // 背景颜色
    painter.drawRoundedRect(QRect(5,  5, width(), height()), 10, 10);  // 背景位置和大小
}