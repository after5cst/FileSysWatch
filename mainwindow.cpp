#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileSystemWatcher>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), watcher(nullptr)
{
    ui->setupUi(this);

    connect(ui->action_Open, &QAction::triggered, this,
            &MainWindow::slotOpenWatch);
    connect(ui->action_Close, &QAction::triggered, this,
            &MainWindow::slotCloseWatch);

    slotCloseWatch();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::slotOpenWatch()
{
    QString path = QFileDialog::getExistingDirectory(
        this, tr("Watch Directory"), "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (path.isEmpty())
    {
        return;
    }

    slotCloseWatch();

    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this,
            &MainWindow::slotDirectoryChanged);
    connect(watcher, &QFileSystemWatcher::fileChanged, this,
            &MainWindow::slotFileChanged);
    watcher->addPath(path);

    slotDirectoryChanged(path);
    ui->listWidget->show();
    ui->plainTextEdit->show();
}

void MainWindow::slotCloseWatch()
{
    if (nullptr != watcher)
    {
        watcher->deleteLater();
    }

    ui->listWidget->hide();
    ui->listWidget->clear();

    ui->plainTextEdit->hide();
    ui->plainTextEdit->clear();
}

void MainWindow::slotDirectoryChanged(const QString &path)
{
    auto watchedFiles = watcher->files();
    watchedFiles.sort();

    QDir dir(path);
    auto allFiles = dir.entryList(QDir::Files, QDir::Name);
    for (auto& file : allFiles)
    {
        file = dir.absoluteFilePath(file);
    }

    auto iterWatch = watchedFiles.begin();
    auto iterAll = allFiles.begin();
    while (!(iterWatch == watchedFiles.end() && iterAll == allFiles.end()))
    {
        if (iterWatch == watchedFiles.end())
        {
            // TODO : Remove watch on file.
            ++iterAll;
        }
        else if (iterAll == allFiles.end())
        {
            // TODO : Add watch on file.
            ++iterWatch;
        }
        else if ((*iterAll) < (*iterWatch))
        {
            // TODO : Remove watch on file.
            ++iterAll;
        }
        else if ((*iterAll) > (*iterWatch))
        {
            // TODO : Add watch on file.
            ++iterWatch;
        }
        else // items match
        {
            ++iterAll;
            ++iterWatch;
        }
    }
}

void MainWindow::slotFileChanged(const QString &path) {}

