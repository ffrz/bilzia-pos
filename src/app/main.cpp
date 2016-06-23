#include "mainwindow.h"

#include <QTimer>
#include <QApplication>
#include <QSqlDatabase>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationDisplayName("Bilzia Point of Sales");

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("bilzia-pos.sqlite3");
        db.open();
    }

    MainWindow mainWindow;

    QTimer::singleShot(0, &mainWindow, SLOT(showMaximized()));

    int exitCode = app.exec();

    {
        QSqlDatabase::database(QSqlDatabase::defaultConnection).close();
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }

    return exitCode;
}
