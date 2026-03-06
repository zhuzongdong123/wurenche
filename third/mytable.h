#ifndef MYTABLE_H
#define MYTABLE_H

#include <QTableWidget>

class MyTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTable(QWidget *parent = nullptr);

signals:

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MYTABLE_H
