#ifndef SALESORDERMANAGER_H
#define SALESORDERMANAGER_H

#include <QSplitter>

class QTabWidget;
class QTableView;
class QLineEdit;
class QLabel;
class QComboBox;

class SalesOrderEditor;
class SalesOrderModel;
class SalesOrderProxyModel;

class SalesOrderManager : public QSplitter
{
    Q_OBJECT
public:
    SalesOrderManager(QWidget* parent);

public slots:
    void refresh();
    void openEditor(qlonglong id = 0);

private slots:
    void edit();
    void applyFilter();
    void init();
    void closeTab(int index);
    void closeCurrentTab();
    void closeAllTabs();
    void onAdded(qlonglong id);
    void onSaved(qlonglong id);
    void onRemoved(qlonglong id);

private:

    QTabWidget* tabWidget;
    QTableView* view;
    QLineEdit* searchEdit;
    QLabel* infoLabel;
    QComboBox* stateComboBox;

    SalesOrderProxyModel* proxyModel;
    SalesOrderModel* model;
    QHash<qlonglong,SalesOrderEditor*> editorById;
};

#endif // SALESORDERMANAGER_H
