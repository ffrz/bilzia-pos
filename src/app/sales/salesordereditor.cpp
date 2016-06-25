#include "salesordereditor.h"

#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QToolBar>
#include <QBoxLayout>
#include <QFormLayout>
#include <QAction>
#include <QLabel>
#include <QGroupBox>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QAbstractTableModel>

class SalesOrderEditor::Model : public QAbstractTableModel
{
public:
    qlonglong orderId;

    enum Column {
        IdColumn,
        NameColumn,
        CostColumn,
        QuantityColumn,
        PriceColumn,
        SubTotalColumn
    };

    struct Item
    {
        inline Item()
            : id(0)
            , quantity(0)
            , cost(0.0)
            , price(0.0)
        {}

        qlonglong id;
        QString name;
        int quantity;
        double cost;
        double price;
    };
    QList<Item> items;

    Model(qlonglong orderId, QObject* parent)
        : QAbstractTableModel(parent)
        , orderId(orderId)
    {}

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags f(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        if ((index.row() == rowCount() - 1 && index.column() == NameColumn)
            || (index.row() < rowCount() - 1 && (index.column() != IdColumn && index.column() != SubTotalColumn)))
            f |= Qt::ItemIsEditable;

        return f;
    }

    int rowCount(const QModelIndex & = QModelIndex()) const
    {
        return items.size() + 1;
    }

    int columnCount(const QModelIndex & = QModelIndex()) const
    {
        return 6;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (index.row() == rowCount() - 1)
            return QVariant();

        const Item item = items.at(index.row());

        if (role == Qt::DisplayRole) {
            QLocale locale;
            switch (index.column()) {
            case CostColumn: return locale.toString(item.cost, 'f', 0);
            case QuantityColumn: return locale.toString(item.quantity);
            case PriceColumn: return locale.toString(item.price, 'f', 0);
            case SubTotalColumn: return locale.toString(item.quantity * item.price, 'f', 0);
            }
        }

        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
            case IdColumn: return item.id;
            case NameColumn: return item.name;
            case CostColumn: return item.cost;
            case QuantityColumn: return item.quantity;
            case PriceColumn: return item.price;
            case SubTotalColumn: return item.quantity * item.price;
            default: return QVariant();
            }
        }
        else if (role == Qt::TextAlignmentRole) {
            switch (index.column()) {
            case IdColumn: return Qt::AlignVCenter ^ Qt::AlignRight;
            case CostColumn: return Qt::AlignVCenter ^ Qt::AlignRight;
            case QuantityColumn: return Qt::AlignVCenter ^ Qt::AlignRight;
            case PriceColumn: return Qt::AlignVCenter ^ Qt::AlignRight;
            case SubTotalColumn: return Qt::AlignVCenter ^ Qt::AlignRight;
            default: return Qt::AlignVCenter ^ Qt::AlignLeft;
            }
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
    {
        if (orientation == Qt::Horizontal) {
            if (role == Qt::DisplayRole) {
                switch (section) {
                case IdColumn: return "ID";
                case NameColumn: return "Nama Produk";
                case CostColumn: return "Modal";
                case QuantityColumn: return "Kwantitas";
                case PriceColumn: return "Harga";
                case SubTotalColumn: return "Sub Total";
                }
            }
        }

        if (role == Qt::DisplayRole) {
            if (section == rowCount() - 1)
                return "*";
            return section + 1;
        }

        return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
        if (role != Qt::EditRole)
            return false;

        if (index.column() == NameColumn) {
            QString name = value.toString().trimmed();
            if (name.isEmpty())
                return false;

            if (index.row() == rowCount() - 1) {
                int row = rowCount();
                beginInsertRows(QModelIndex(), row, row);
                Item item;
                item.name = name;
                items.append(item);
                endInsertRows();
            }
            else {
                items[index.row()].name = name;
                emit dataChanged(index, index);
            }
            return true;
        }

        Item &item = items[index.row()];
        if (index.column() == CostColumn) {
            item.cost = QLocale().toDouble(value.toString());
        }
        else if (index.column() == QuantityColumn) {
            item.quantity = QLocale().toInt(value.toString());
            QModelIndex subTotalIndex = index.sibling(index.row(), SubTotalColumn);
            emit dataChanged(subTotalIndex, subTotalIndex);
        }
        else if (index.column() == PriceColumn) {
            item.price = QLocale().toInt(value.toString());
            QModelIndex subTotalIndex = index.sibling(index.row(), SubTotalColumn);
            emit dataChanged(subTotalIndex, subTotalIndex);
        }

        emit dataChanged(index, index);

        return true;
    }
};

SalesOrderEditor::SalesOrderEditor(qlonglong id, QWidget* parent)
    : QWidget(parent)
    , id(id)
    , model(new Model(id, this))
{
    QToolBar* toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16, 16));

    QString actionToolTip("%1<br><b>%2</b>");

    QAction* saveAction = toolBar->addAction(QIcon("_r/icons/document-save.png"), "", this, SLOT(save()));
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAction->setToolTip(actionToolTip.arg("Simpan order").arg("Ctrl+S"));

    toolBar->addSeparator();

    QAction* printAction = toolBar->addAction(QIcon("_r/icons/document-print.png"), "", this, SLOT(print()));
    printAction->setShortcut(QKeySequence("Ctrl+P"));
    printAction->setToolTip(actionToolTip.arg("Cetak pesanan").arg("Ctrl+P"));

    QAction* printPreviewAction = toolBar->addAction(QIcon("_r/icons/document-print-preview.png"), "", this, SLOT(printPreview()));
    printPreviewAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    printPreviewAction->setToolTip(actionToolTip.arg("Pratinjau cetak pesanan").arg("Ctrl+Shift+P"));

    toolBar->addSeparator();

    QAction* removeAction = toolBar->addAction(QIcon("_r/icons/document-close.png"), "", this, SLOT(remove()));
    removeAction->setShortcut(QKeySequence("Ctrl+Shift+Del"));
    removeAction->setToolTip(actionToolTip.arg("Hapus rekaman pesanan").arg("Ctrl+Shift+Del"));

    QGroupBox* orderInfoGroupBox = new QGroupBox("Pesanan", this);
    QFormLayout* orderInfoLayout = new QFormLayout(orderInfoGroupBox);

    idEdit = new QLineEdit(orderInfoGroupBox);
    idEdit->setReadOnly(true);
    idEdit->setPlaceholderText("Otomatis");
    orderInfoLayout->addRow("&Nomor", idEdit);

    openDateTimeEdit = new QDateTimeEdit(orderInfoGroupBox);
    openDateTimeEdit->setCalendarPopup(true);
    openDateTimeEdit->setDisplayFormat("dd/MM/yyyy");
    orderInfoLayout->addRow("&Tanggal", openDateTimeEdit);

    stateComboBox = new QComboBox(orderInfoGroupBox);
    stateComboBox->addItem("Aktif");
    stateComboBox->addItem("Selesai");
    stateComboBox->addItem("Dibatalkan");
    orderInfoLayout->addRow("&Status", stateComboBox);

    QGroupBox* customerInfoGroupBox = new QGroupBox("Pelanggan", this);
    QFormLayout* customerInfoLayout = new QFormLayout(customerInfoGroupBox);

    customerNameEdit = new QLineEdit(customerInfoGroupBox);
    customerNameEdit->setMaxLength(100);
    customerInfoLayout->addRow("&Nama", customerNameEdit);

    customerContactEdit = new QLineEdit(customerInfoGroupBox);
    customerContactEdit->setMaxLength(100);
    customerInfoLayout->addRow("&Kontak", customerContactEdit);

    customerAddressEdit = new QLineEdit(customerInfoGroupBox);
    customerAddressEdit->setMaxLength(100);
    customerInfoLayout->addRow("&Alamat", customerAddressEdit);

    QBoxLayout* layout1 = new QHBoxLayout;
    layout1->addWidget(customerInfoGroupBox);
    layout1->addWidget(orderInfoGroupBox);

    view = new QTableView(this);
    view->setModel(model);
    view->setCornerButtonEnabled(false);
    view->setAlternatingRowColors(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setTabKeyNavigation(false);
    view->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
    QHeaderView* header = view->horizontalHeader();
    header->setHighlightSections(false);
    header = view->verticalHeader();
    header->setHighlightSections(false);
    header->setDefaultSectionSize(20);
    header->setMinimumSectionSize(20);
    header->setMaximumSectionSize(20);

    infoLabel = new QLabel(this);
    infoLabel->setStyleSheet("font-style:italic;padding-bottom:1px;");
    infoLabel->setText("Belum disimpan");

    totalEdit = new QLineEdit(this);
    totalEdit->setReadOnly(true);
    totalEdit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    QFormLayout* totalLayout = new QFormLayout;
    totalLayout->addRow("&Grand Total", totalEdit);

    QBoxLayout* footerLayout = new QHBoxLayout;
    footerLayout->addWidget(infoLabel);
    footerLayout->addStretch(1);
    footerLayout->addLayout(totalLayout);

    QBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->addWidget(toolBar);
    mainLayout->addLayout(layout1);
    mainLayout->addWidget(view);
    mainLayout->addLayout(footerLayout);

    if (id == 0) {
        printAction->setEnabled(false);
        printPreviewAction->setEnabled(false);
        removeAction->setEnabled(false);
        openDateTimeEdit->setDateTime(QDateTime::currentDateTime());
        stateComboBox->setCurrentIndex(0);
        totalEdit->setText("0");
    }
    else {
        QSqlQuery q;
        q.prepare("select * from sales_orders where id=?");
        q.bindValue(0, id);
        q.exec();
        q.next();
        idEdit->setText(QString::number(q.value("id").toLongLong()));
        openDateTimeEdit->setDateTime(q.value("open_datetime").toDateTime());
        stateComboBox->setCurrentIndex(q.value("state").toInt());
        customerNameEdit->setText(q.value("customer_name").toString());
        customerContactEdit->setText(q.value("customer_contact").toString());
        customerAddressEdit->setText(q.value("customer_address").toString());
        totalEdit->setText(QLocale().toString(q.value("grand_total").toDouble(), 'f', 0));
    }

    updateWindowTitle();
    QTimer::singleShot(0, this, SLOT(init()));
}

void SalesOrderEditor::init()
{
    customerNameEdit->setFocus();

    QHeaderView* header = view->horizontalHeader();
    header->hideSection(Model::IdColumn);
    header->setSectionResizeMode(Model::NameColumn, QHeaderView::Stretch);
}

void SalesOrderEditor::updateWindowTitle()
{
    setWindowTitle(id ? QString("#%1").arg(QString::number(id)) : "Baru");
}

void SalesOrderEditor::save()
{
    const QString customerName = customerNameEdit->text().trimmed();
    if (customerName.isEmpty()) {
        customerNameEdit->setFocus();
        QMessageBox::warning(0, "Peringatan", "Nama pelanggan harus diisi.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery q(db);
    QString sql;
    if (!id) {
        sql = "insert into sales_orders("
              " open_datetime, state,"
              " customer_name, customer_contact, customer_address,"
              " grand_total"
              ") values ("
              ":open_datetime,:state,"
              ":customer_name,:customer_contact,:customer_address,"
              ":grand_total"
              ")";
    }
    else {
        sql = "update sales_orders set"
              " open_datetime=:open_datetime"
              ",state=:state"
              ",customer_name=:customer_name"
              ",customer_contact=:customer_contact"
              ",customer_address=:customer_address"
              ",grand_total=:grand_total"
              " where id=:id";
    }
    q.prepare(sql);
    q.bindValue(":open_datetime", openDateTimeEdit->dateTime());
    q.bindValue(":state", stateComboBox->currentIndex());
    q.bindValue(":customer_name", customerName);
    q.bindValue(":customer_contact", customerContactEdit->text().trimmed());
    q.bindValue(":customer_address", customerAddressEdit->text().trimmed());
    q.bindValue(":grand_total", QLocale().toDouble(totalEdit->text()));

    if (id)
        q.bindValue(":id", id);

    q.exec();

    bool emitAddedSignal = false;
    if (!id) {
        id = q.lastInsertId().toLongLong();
        idEdit->setText(QString::number(id));
        updateWindowTitle();
        emitAddedSignal = true;
    }

    if (!db.commit())
        db.rollback();

    updateWindowTitle();

    if (emitAddedSignal)
        emit added(id);

    emit saved(id);
}

void SalesOrderEditor::print()
{

}

void SalesOrderEditor::printPreview()
{
}

void SalesOrderEditor::remove()
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery q(db);
    q.prepare("delete from sales_orders where id=?");
    q.bindValue(0, id);
    q.exec();

    db.commit();

    emit removed(id);
}

