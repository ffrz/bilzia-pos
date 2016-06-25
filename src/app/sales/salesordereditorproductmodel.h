#ifndef SALESORDEREDITORPRODUCTMODEL_H
#define SALESORDEREDITORPRODUCTMODEL_H

#include <QSqlQueryModel>
#include "salesordereditor.h"

class SalesOrderEditor::ProductModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    ProductModel(QObject*parent);

    static inline ProductModel* instance() { return self; }

public slots:
    void refresh();

private:
    static ProductModel* self;
};

#endif // SALESORDEREDITORPRODUCTMODEL_H
