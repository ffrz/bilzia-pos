#ifndef SALESORDEREDITOR_H
#define SALESORDEREDITOR_H

#include <QWidget>

class SalesOrderEditor : public QWidget
{
    Q_OBJECT
public:
    SalesOrderEditor(qlonglong id, QWidget* parent);

public:
    qlonglong id;

private:
    void updateWindowTitle();
};

#endif // SALESORDEREDITOR_H
