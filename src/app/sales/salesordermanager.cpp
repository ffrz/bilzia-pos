#include "salesordermanager.h"
#include "salesordereditor.h"
#include "salesordermodel.h"
#include "salesorderproxymodel.h"

#include <QTimer>
#include <QTabWidget>
#include <QBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

SalesOrderManager::SalesOrderManager(QWidget* parent)
    : QSplitter(parent)
{
    model = new SalesOrderModel(this);
    proxyModel = new SalesOrderProxyModel(this);
    proxyModel->setSourceModel(model);

    QWidget* container = new QWidget(this);
    QBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setMargin(0);
    containerLayout->setSpacing(0);

    QString actionTooltip("%1<br><b>%2</b>");
    QToolBar* toolBar = new QToolBar(container);
    QAction* refreshAction = toolBar->addAction("&Muat Ulang", this, SLOT(refresh()));
    refreshAction->setShortcut(QKeySequence("F5"));
    refreshAction->setToolTip(actionTooltip.arg("Muat ulang daftar pesanan").arg(refreshAction->shortcut().toString()));

    QAction* addAction = toolBar->addAction("&Baru", this, SLOT(openEditor()));
    addAction->setShortcut(QKeySequence("Ctrl+N"));
    addAction->setToolTip(actionTooltip.arg("Pesanan baru").arg(addAction->shortcut().toString()));

    containerLayout->addWidget(toolBar);

    QLabel* spacer = new QLabel(toolBar);
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    toolBar->addWidget(spacer);

    stateComboBox = new QComboBox(toolBar);
    stateComboBox->setToolTip("Saring daftar pesanan berdasarkan status");
    stateComboBox->addItem("Semua");
    stateComboBox->addItem("Aktif");
    stateComboBox->addItem("Selesai");
    stateComboBox->addItem("Dibatalkan");
    stateComboBox->setCurrentIndex(1);
    toolBar->addWidget(stateComboBox);

    searchEdit = new QLineEdit(toolBar);
    searchEdit->setToolTip("Cari di daftar pesanan");
    searchEdit->setPlaceholderText("Cari");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setMaxLength(100);
    searchEdit->setMaximumWidth(150);
    toolBar->addWidget(searchEdit);

    view = new QTableView(container);
    view->setModel(proxyModel);
    view->setAlternatingRowColors(true);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setTabKeyNavigation(false);
    QHeaderView* header = view->verticalHeader();
    header->setVisible(false);
    header->setMinimumSectionSize(20);
    header->setMaximumSectionSize(20);
    header->setDefaultSectionSize(20);
    header = view->horizontalHeader();
    header->setHighlightSections(false);

    containerLayout->addWidget(view);

    infoLabel = new QLabel(container);
    infoLabel->setStyleSheet("font-style:italic;padding:1px 0 1px 0;");
    infoLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    containerLayout->addWidget(infoLabel);

    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->hide();

    setCollapsible(0, false);
    setCollapsible(1, false);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(stateComboBox, SIGNAL(currentIndexChanged(int)), SLOT(refresh()));
    connect(searchEdit, SIGNAL(textChanged(QString)), SLOT(applyFilter()));
    connect(view, SIGNAL(activated(QModelIndex)), SLOT(edit()));

    QTimer::singleShot(0, this, SLOT(init()));
}

void SalesOrderManager::init()
{
    refresh();
    view->sortByColumn(0, Qt::AscendingOrder);
}

void SalesOrderManager::refresh()
{
    int state = stateComboBox->currentIndex() - 1;

    model->refreshAll(state);
    applyFilter();

    view->resizeColumnsToContents();
    view->horizontalHeader()->setStretchLastSection(true);
}

void SalesOrderManager::applyFilter()
{
    QString query = searchEdit->text().trimmed();
    if (query.isEmpty())
        proxyModel->setFilterFixedString(query);
    else
        proxyModel->setFilterWildcard("*" + query + "*");

    QString info;
    if (model->rowCount() == 0)
        info = "Tidak ada rekaman yang dapat ditampilkan";
    else if (model->rowCount() == proxyModel->rowCount())
        info = QString("Menampilkan %1 rekaman").arg(model->rowCount());
    else
        info = QString("Menampilkan %1 rekaman disaring dari total %2 rekaman").arg(proxyModel->rowCount()).arg(model->rowCount());

    infoLabel->setText(info);
}

void SalesOrderManager::edit()
{
    openEditor(view->selectionModel()->selectedRows(SalesOrderModel::IdColumn).first().data(Qt::EditRole).toLongLong());
}

void SalesOrderManager::openEditor(qlonglong id)
{
    SalesOrderEditor* editor = 0;

    if (id > 0)
        editor = editorById.value(id);

    if (!editor) {
        editor = new SalesOrderEditor(id, tabWidget);
        tabWidget->addTab(editor, editor->windowTitle());

        if (id != 0)
            editorById.insert(id, editor);
    }

    tabWidget->setCurrentWidget(editor);

    if (tabWidget->isHidden())
        tabWidget->show();
}

void SalesOrderManager::closeTab(int index)
{
    SalesOrderEditor* editor = static_cast<SalesOrderEditor*>(tabWidget->widget(index));
    if (!editor->close())
        return;

    tabWidget->removeTab(index);

    if (editor->id != 0)
        editorById.remove(editor->id);

    delete editor;

    if (tabWidget->count() == 0)
        tabWidget->hide();
}

void SalesOrderManager::closeCurrentTab()
{
    if (tabWidget->count() == 0)
        return;

    closeTab(tabWidget->currentIndex());
}

void SalesOrderManager::closeAllTabs()
{
    for (int i = tabWidget->count() - 1; i >= 0; i--)
        closeTab(i);
}
