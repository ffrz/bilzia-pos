#include "salesordereditorproductmodel.h"

SalesOrderEditor::ProductModel* SalesOrderEditor::ProductModel::self = 0;

SalesOrderEditor::ProductModel::ProductModel(QObject*parent)
    : QSqlQueryModel(parent)
{
    self = this;
}

void SalesOrderEditor::ProductModel::refresh()
{
    setQuery("select name from products order by name asc");
}
