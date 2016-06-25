#include "mainwindow.h"
#include "sales/salesordereditorproductmodel.h"

#include <QTimer>
#include <QApplication>
#include <QSqlDatabase>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("Bilzia Point of Sales");

    QLocale::setDefault(QLocale(QLocale::Indonesian, QLocale::Indonesia));

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("bilzia-pos.sqlite3");
        db.open();
    }

    MainWindow mainWindow;

    QTimer::singleShot(0, &mainWindow, SLOT(showMaximized()));
    QTimer::singleShot(0, new SalesOrderEditor::ProductModel(&app), SLOT(refresh()));

    int exitCode = app.exec();

    {
        QSqlDatabase::database(QSqlDatabase::defaultConnection).close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    return exitCode;
}
