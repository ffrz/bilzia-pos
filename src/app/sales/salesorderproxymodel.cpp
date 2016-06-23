#include "salesorderproxymodel.h"

SalesOrderProxyModel::SalesOrderProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(-1);
    setFilterRole(Qt::DisplayRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(Qt::DisplayRole);
}
