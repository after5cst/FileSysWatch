#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->action_Open, &QAction::triggered, this,
            &MainWindow::SlotOpenWatch);
    connect(ui->action_Close, &QAction::triggered, this,
            &MainWindow::SlotCloseWatch);

    SlotCloseWatch();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::SlotOpenWatch()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Watch Directory"), "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
    {
        return;
    }

    SlotCloseWatch();

    // TODO : Start watching directory here

    ui->listWidget->show();
    ui->plainTextEdit->show();
}

void MainWindow::SlotCloseWatch()
{
    ui->listWidget->hide();
    ui->listWidget->clear();

    ui->plainTextEdit->hide();
    ui->plainTextEdit->clear();
}
