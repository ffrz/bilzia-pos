#include "mainwindow.h"
#include "sales/salesordermanager.h"

MainWindow::MainWindow()
{
    salesOrderManager = new SalesOrderManager(this);
    setCentralWidget(salesOrderManager);
}
