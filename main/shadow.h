#ifndef SHADOW_H
#define SHADOW_H

#include <QWidget>

class shadow : public QWidget
{
    Q_OBJECT
public:
    explicit shadow(QWidget *parent = nullptr);

signals:

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // SHADOW_H
