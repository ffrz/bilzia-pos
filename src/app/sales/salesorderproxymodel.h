#ifndef SALESORDERPROXYMODEL_H
#define SALESORDERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SalesOrderProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SalesOrderProxyModel(QObject* parent);
};

#endif // SALESORDERPROXYMODEL_H
