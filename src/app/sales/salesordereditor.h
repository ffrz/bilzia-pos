#ifndef SALESORDEREDITOR_H
#define SALESORDEREDITOR_H

#include <QWidget>

class QLabel;
class QTableView;
class QDateTimeEdit;
class QComboBox;
class QLineEdit;

class SalesOrderEditor : public QWidget
{
    Q_OBJECT
public:
    class Model;
    class Delegate;
    class ProductModel;
    SalesOrderEditor(qlonglong id, QWidget* parent);

signals:
    void added(qlonglong id);
    void saved(qlonglong id);
    void removed(qlonglong id);
    void closeRequest();

public slots:
    void init();
    void save();
    void remove();
    void removeCurrentItem();
    void print();
    void printPreview();
    void updateTotal();

public:
    qlonglong id;

private:
    void updateWindowTitle();
    void setInfoLabel(const QDateTime& lastmod);

    QLabel* infoLabel;
    QTableView* view;
    QLineEdit* customerNameEdit;
    QLineEdit* customerContactEdit;
    QLineEdit* customerAddressEdit;
    QLineEdit* idEdit;
    QDateTimeEdit* openDateTimeEdit;
    QComboBox* stateComboBox;
    QLineEdit* totalEdit;

    Model* model;
    Delegate* delegate;
};

#endif // SALESORDEREDITOR_H
