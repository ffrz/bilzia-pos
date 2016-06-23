#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SalesOrderManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

private:
    SalesOrderManager* salesOrderManager;
};

#endif // MAINWINDOW_H
