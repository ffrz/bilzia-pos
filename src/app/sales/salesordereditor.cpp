#include "salesordereditor.h"
#include "salesordereditorproductmodel.h"

#include <QMessageBox>
#include <QColor>
#include <QTimer>
#include <QSqlQuery>
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
#include <QStyledItemDelegate>
#include <QCompleter>

class SalesOrderEditor::Model : public QAbstractTableModel
{
    Q_OBJECT
public:
    qlonglong orderId;
    double total;

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
    QList<qlonglong> deletedIds;

signals:
    void totalChanged();

public:
    Model(qlonglong orderId, QObject* parent)
        : QAbstractTableModel(parent)
        , orderId(orderId)
        , total(0)
    {
        if (orderId) {
            QSqlQuery q;
            q.prepare("select * from sales_order_details where parent_id=?");
            q.bindValue(0, orderId);
            q.exec();
            while (q.next()) {
                Item item;
                item.id = q.value("id").toLongLong();
                item.name = q.value("name").toString();
                item.quantity = q.value("quantity").toInt();
                item.cost = q.value("cost").toDouble();
                item.price = q.value("price").toDouble();
                items.append(item);
            }
        }
    }

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
        else if (role == Qt::BackgroundColorRole) {
            if (item.price == item.cost)
                return QColor("#ffffdd");
            else if (item.price < item.cost) {
                return QColor("#ffdddd");
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
            QString name = value.toString();
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
            double cost = value.toDouble();
            if (item.cost == cost)
                return true;

            if (item.price != 0.0 && cost > item.price) {
                QMessageBox::warning(0, "Peringatan", "Modal lebih besar dari harga, silahkan perbarui harga.");
            }

            item.cost = cost;
        }
        else if (index.column() == QuantityColumn) {
            int quantity = value.toInt();
            if (item.quantity == quantity)
                return true;
            item.quantity = quantity;
            QModelIndex subTotalIndex = index.sibling(index.row(), SubTotalColumn);
            emit dataChanged(subTotalIndex, subTotalIndex);
            updateTotal();
        }
        else if (index.column() == PriceColumn) {
            double price = value.toDouble();
            if (item.price == price)
                return true;
            if (price < item.cost && confirmNegativeProfit())
                return false;

            item.price = price;
            QModelIndex subTotalIndex = index.sibling(index.row(), SubTotalColumn);
            emit dataChanged(subTotalIndex, subTotalIndex);
            updateTotal();
        }

        emit dataChanged(index, index);

        return true;
    }

    void updateTotal()
    {
        total = 0.;
        for (const Item item: items) {
            total += item.quantity * item.price;
        }
        emit totalChanged();
    }

    void save() {
        QSqlQuery q;

        for (qlonglong id: deletedIds) {
            q.prepare("delete from sales_order_details where id=?");
            q.bindValue(0, id);
            q.exec();
        }

        for (Item& item: items) {
            if (item.id == 0) {
                q.prepare("insert into sales_order_details("
                          " parent_id, name, quantity, cost, price, profit"
                          ")values("
                          ":parent_id,:name,:quantity,:cost,:price,:profit"
                          ")");
                q.bindValue(":parent_id", orderId);
            }
            else {
                q.prepare("update sales_order_details set"
                          " name=:name"
                          ",quantity=:quantity"
                          ",cost=:cost"
                          ",price=:price"
                          ",profit=:profit"
                          " where id=:id");
                q.bindValue(":id", item.id);
            }

            q.bindValue(":name", item.name);
            q.bindValue(":cost", item.cost);
            q.bindValue(":price", item.price);
            q.bindValue(":quantity", item.quantity);
            q.bindValue(":profit", (item.quantity * item.price) - item.quantity * item.cost);
            q.exec();

            if (item.id == 0) {
                item.id = q.lastInsertId().toLongLong();
            }

            q.prepare("insert or ignore into products (name) values (:name)");
            q.bindValue(":name", item.name);
            q.exec();
        }

        ProductModel::instance()->refresh();
    }

    bool removeRows(int row, int /*count*/, const QModelIndex &parent = QModelIndex())
    {
        beginRemoveRows(parent, row, row);
        Item item = items.takeAt(row);
        if (item.id != 0)
            deletedIds.append(item.id);
        endRemoveRows();
        return true;
    }

    bool confirmNegativeProfit() const
    {
        return QMessageBox::question(0, "Konfirmasi", "Harga lebih kecil dari modal, lanjutkan perubahan?", "&Ya", "&Tidak");
    }
};

class SalesOrderEditor::Delegate : public QStyledItemDelegate
{
public:
    Delegate(QObject* parent)
        : QStyledItemDelegate(parent)
    {
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (index.column() == Model::NameColumn) {
            QLineEdit* editor = new QLineEdit(parent);
            editor->setMaxLength(100);
            editor->setFrame(false);

            QCompleter* completer = new QCompleter(editor);
            completer->setModel(ProductModel::instance());
            completer->setCaseSensitivity(Qt::CaseInsensitive);
            completer->setFilterMode(Qt::MatchContains);
            completer->setCompletionMode(QCompleter::PopupCompletion);
            completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
            completer->popup()->setAlternatingRowColors(true);

            editor->setCompleter(completer);
            return editor;
        }
        else if (index.column() == Model::CostColumn || index.column() == Model::QuantityColumn || index.column() == Model::PriceColumn) {
            QLineEdit* editor = new QLineEdit(parent);
            QRegExpValidator* numberValidator = new QRegExpValidator(QRegExp("^((?:\\d+|\\d{1,3}(?:\\.\\d{3})*)?)$"), editor);
            editor->setValidator(numberValidator);
            editor->setMaxLength(10);
            editor->setFrame(false);
            editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            return editor;
        }

        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget* pEditor, const QModelIndex& index) const
    {
        if (index.column() == Model::NameColumn) {
            QLineEdit* editor = static_cast<QLineEdit*>(pEditor);
            editor->setText(index.data(Qt::EditRole).toString());
        }
        else if (index.column() == Model::CostColumn || index.column() == Model::QuantityColumn || index.column() == Model::PriceColumn) {
            QLineEdit* editor = static_cast<QLineEdit*>(pEditor);
            editor->setText(QLocale().toString(index.data(Qt::EditRole).toDouble(), 'f', 0));
        }
    }

    void setModelData(QWidget* pEditor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        if (index.column() == Model::NameColumn) {
            QLineEdit* editor = static_cast<QLineEdit*>(pEditor);
            model->setData(index, editor->text().trimmed());
        }
        else if (index.column() == Model::CostColumn || index.column() == Model::QuantityColumn || index.column() == Model::PriceColumn) {
            QLineEdit* editor = static_cast<QLineEdit*>(pEditor);
            model->setData(index, QLocale().toDouble(editor->text()));
        }
    }

};

SalesOrderEditor::SalesOrderEditor(qlonglong id, QWidget* parent)
    : QWidget(parent)
    , id(id)
    , model(new Model(id, this))
    , delegate(new Delegate(this))
{
    QToolBar* toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(16, 16));

    QString actionToolTip("%1<br><b>%2</b>");

    QAction* saveAction = toolBar->addAction(QIcon("_r/icons/save.png"), "", this, SLOT(save()));
    saveAction->setShortcut(QKeySequence("Ctrl+S"));
    saveAction->setToolTip(actionToolTip.arg("Simpan order").arg("Ctrl+S"));

    toolBar->addSeparator();

    QAction* printAction = toolBar->addAction(QIcon("_r/icons/document-print.png"), "", this, SLOT(print()));
    printAction->setShortcut(QKeySequence("Ctrl+P"));
    printAction->setToolTip(actionToolTip.arg("Cetak pesanan").arg("Ctrl+P"));

    QAction* printPreviewAction = toolBar->addAction(QIcon("_r/icons/preview.png"), "", this, SLOT(printPreview()));
    printPreviewAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    printPreviewAction->setToolTip(actionToolTip.arg("Pratinjau cetak pesanan").arg("Ctrl+Shift+P"));

    toolBar->addSeparator();

    QAction* removeAction = toolBar->addAction(QIcon("_r/icons/remove.png"), "", this, SLOT(remove()));
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
    view->setItemDelegate(delegate);
    view->setCornerButtonEnabled(false);
    view->setAlternatingRowColors(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setTabKeyNavigation(false);
    view->setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
    view->viewport()->setToolTip("Daftar produk. Ketuk tombol <b>Del</b> untuk menghapus produk yang dipilih.");
    QHeaderView* header = view->horizontalHeader();
    header->setHighlightSections(false);
    header = view->verticalHeader();
    header->setHighlightSections(false);
    header->setDefaultSectionSize(20);
    header->setMinimumSectionSize(20);
    header->setMaximumSectionSize(20);

    QAction* removeItemAction = new QAction(view);
    removeItemAction->setShortcut(QKeySequence("Del"));
    view->addAction(removeItemAction);

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
        setInfoLabel(q.value("lastmod_datetime").toDateTime());
    }

    connect(model, SIGNAL(totalChanged()), SLOT(updateTotal()));
    connect(removeItemAction, SIGNAL(triggered(bool)), SLOT(removeCurrentItem()));

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

    const QDateTime now = QDateTime::currentDateTime();

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery q(db);
    QString sql;
    if (!id) {
        sql = "insert into sales_orders("
              " open_datetime, state,"
              " customer_name, customer_contact, customer_address,"
              " grand_total,"
              " lastmod_datetime"
              ") values ("
              ":open_datetime,:state,"
              ":customer_name,:customer_contact,:customer_address,"
              ":grand_total,"
              ":lastmod_datetime"
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
              ",lastmod_datetime=:lastmod_datetime"
              " where id=:id";
    }
    q.prepare(sql);
    q.bindValue(":open_datetime", openDateTimeEdit->dateTime());
    q.bindValue(":state", stateComboBox->currentIndex());
    q.bindValue(":customer_name", customerName);
    q.bindValue(":customer_contact", customerContactEdit->text().trimmed());
    q.bindValue(":customer_address", customerAddressEdit->text().trimmed());
    q.bindValue(":grand_total", QLocale().toDouble(totalEdit->text()));
    q.bindValue(":lastmod_datetime", now);

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

    model->save();

    if (!db.commit())
        db.rollback();

    setInfoLabel(now);
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
    if (QMessageBox::question(0, "Konfirmasi", QString("Hapus transaksi nomor %1?").arg(id), "&Ya", "&Tidak"))
        return;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery q(db);
    q.prepare("delete from sales_orders where id=?");
    q.bindValue(0, id);
    q.exec();

    q.prepare("delete from sales_order_details where parent_id=?");
    q.bindValue(0, id);
    q.exec();

    db.commit();

    emit removed(id);
}

void SalesOrderEditor::updateTotal()
{
    totalEdit->setText(QLocale().toString(model->total, 'f', 0));
}

void SalesOrderEditor::removeCurrentItem()
{
    QModelIndexList indexes = view->selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;

    int row = indexes.first().row();

    if (row == model->rowCount() - 1)
        return;

    if (QMessageBox::question(0, "Konfirmasi", QString("Hapus produk <b>%1</b>?").arg(model->items[row].name), "&Ya", "&Tidak"))
        return;

    model->removeRow(row);
}

void SalesOrderEditor::setInfoLabel(const QDateTime& lastmod)
{
    infoLabel->setText(QString("Terakhir disimpan pada hari %1.").arg(lastmod.toString("dddd, dd MMMM yyyy hh:mm:ss")));
}

#include "salesordereditor.moc"
