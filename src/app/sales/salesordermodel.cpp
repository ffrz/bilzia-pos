#include "salesordermodel.h"

#include <QSqlQuery>
#include <QLocale>
#include <QVariant>
#include <QDateTime>
#include <QColor>

#define SELECT_COLUMNS_FROM_SALES_ORDERS \
    "select id, state, open_datetime, grand_total, customer_name, customer_contact, customer_address "\
    "from sales_orders"

SalesOrderModel::SalesOrderModel(QObject* parent)
    : QAbstractTableModel(parent)
    , stateFilter(-1)
{
}

QVariant SalesOrderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case IdColumn: return "Nomor";
            case StateColumn: return "Status";
            case OpenDateTimeColumn: return "Tanggal";
            case GrandTotalColumn: return "Grand Total";
            case CustomerNameColumn: return "Atas Nama";
            case CustomerContactColumn: return "Kontak";
            case CustomerAddressColumn: return "Alamat";
            }
        }
    }

    return QVariant();
}

int SalesOrderModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 7;
}

int SalesOrderModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : items.size();
}

QVariant SalesOrderModel::data(const QModelIndex& index, int role) const
{
    QVector<QVariant> item = items.at(index.row());

    if (role == Qt::DisplayRole) {
        if (index.column() == GrandTotalColumn)
            return QLocale().toString(item.at(index.column()).toDouble(), 'f', 0);
        else if (index.column() == OpenDateTimeColumn)
            return item.at(index.column()).toDateTime().date().toString("dd/MM/yyyy");
        else if (index.column() == StateColumn) {
            int state = item.at(index.column()).toInt();
            return state == 0 ? "Aktif" : (state == 1 ? "Selesai" : "Dibatalkan");
        }
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return item.at(index.column());
    else if (role == Qt::TextAlignmentRole) {
        if (index.column() == IdColumn || index.column() == GrandTotalColumn)
            return Qt::AlignRight ^ Qt::AlignVCenter;
        else if (index.column() == StateColumn)
            return Qt::AlignCenter;
        return Qt::AlignLeft ^ Qt::AlignVCenter;
    }
    else if (role == Qt::BackgroundColorRole) {
        int state = item.at(StateColumn).toInt();
        return state == 1 ? QColor("#eeffee") : (state == 2 ? QColor("#ffeeee") : QVariant());
    }

    return QVariant();
}

void SalesOrderModel::refreshAll(int pStateFilter)
{
    stateFilter = pStateFilter;
    QString sql = SELECT_COLUMNS_FROM_SALES_ORDERS;

    if (stateFilter >= 0)
        sql.append(" where state=" + QString::number(stateFilter));

    QSqlQuery q;
    q.prepare(sql);
    q.exec();

    beginResetModel();
    items.clear();
    rowById.clear();

    int row = 0;
    while (q.next()) {
        const QVector<QVariant> item = createItem(q);
        items.append(item);
        rowById.insert(item.at(IdColumn).toLongLong(), row);
        row++;
    }

    endResetModel();
}

QVector<QVariant> SalesOrderModel::createItem(QSqlQuery &q)
{
    QVector<QVariant> item(columnCount());
    for (int col = 0; col < columnCount(); col++)
        item[col] = q.value(col);
    return item;
}

void SalesOrderModel::refresh(qlonglong id)
{
    QString sql = SELECT_COLUMNS_FROM_SALES_ORDERS " where id=" + QString::number(id);

    if (stateFilter >= 0)
        sql.append(" and state=" + QString::number(stateFilter));

    QSqlQuery q;
    q.prepare(sql);
    q.exec();
    int row = rowById.value(id, -1);

    if (!q.next()) {
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            rowById.remove(id);
            items.removeAt(row);
            endRemoveRows();
        }
        return;
    }

    if (row == -1) {
        row = rowCount();
        beginInsertRows(QModelIndex(), row, row);
        rowById.insert(id, row);
        items.append(createItem(q));
        endInsertRows();
        return;
    }

    items[row] = createItem(q);
    const QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx.sibling(row, columnCount() - 1));
    return;


}
