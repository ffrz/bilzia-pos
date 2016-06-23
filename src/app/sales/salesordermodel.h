#ifndef SALESORDERMODEL_H
#define SALESORDERMODEL_H

#include <QAbstractTableModel>

class QSqlQuery;

class SalesOrderModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns {
        IdColumn,
        StateColumn,
        OpenDateTimeColumn,
        GrandTotalColumn,
        CustomerNameColumn,
        CustomerContactColumn,
        CustomerAddressColumn
    };

    SalesOrderModel(QObject* parent);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    void refreshAll(int stateFilter);

private:
    QVector<QVariant> createItem(QSqlQuery &q);

    QList<QVector<QVariant>> items;
};

#endif // SALESORDERMODEL_H
