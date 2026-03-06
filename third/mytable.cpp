#include "mytable.h"
#include <QPainter>
MyTable::MyTable(QWidget *parent) : QTableWidget(parent)
{

}

void MyTable::paintEvent(QPaintEvent *event) {
        QTableWidget::paintEvent(event);

        QPainter painter(viewport());
        painter.setPen(QPen(QColor(255,  255, 255, 0), 2)); // 设置网格线颜色为半透明黑色，宽度为1

        // 绘制垂直网格线
        for (int i = 0; i <= columnCount(); ++i) {
            int x = columnViewportPosition(i);
            painter.drawLine(x,  0, x, height());
        }

        // 绘制水平网格线
        for (int i = 0; i <= rowCount(); ++i) {
            int y = rowViewportPosition(i);
            painter.drawLine(0,  y, width(), y);
        }
    }
