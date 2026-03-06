#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QWidget>
#include <QAction>
#include <QPushButton>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = nullptr);

signals:

protected:
    void showEvent(QShowEvent *event);

private:
    QPushButton *m_iconBtn = nullptr;

private slots:
    void slt_keyBoardClicked();
};

#endif // LINEEDIT_H
