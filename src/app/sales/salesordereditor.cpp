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
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QPlainTextEdit>

SalesOrderEditor::SalesOrderEditor(qlonglong id, QWidget* parent)
    : QWidget(parent)
    , id(id)
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

    QSqlQuery q;
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
    QSqlQuery q;
    q.prepare("delete from sales_orders where id=?");
    q.bindValue(0, id);
    q.exec();

    db.transaction();
    db.commit();

    emit removed(id);
}

