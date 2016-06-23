#include "salesordereditor.h"

SalesOrderEditor::SalesOrderEditor(qlonglong id, QWidget* parent)
    : QWidget(parent)
    , id(id)
{
    updateWindowTitle();
}

void SalesOrderEditor::updateWindowTitle()
{
    setWindowTitle(id ? QString("#%1").arg(QString::number(id)) : "Baru");
}
