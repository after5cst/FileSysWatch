#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), watcher(nullptr),
      initialized(false)
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
    initialized = true;
}

void MainWindow::slotCloseWatch()
{
    if (nullptr != watcher)
    {
        watcher->deleteLater();
        watcher = nullptr;
    }

    ui->listWidget->hide();
    ui->listWidget->clear();

    ui->plainTextEdit->hide();
    ui->plainTextEdit->clear();
    initialized = false;
}

void MainWindow::slotDirectoryChanged(const QString &path)
{
    auto watchedFiles = watcher->files();
    watchedFiles.sort();

    QDir dir(path);
    auto allFiles = dir.entryList(QDir::Files, QDir::Name);
    for (auto &file : allFiles)
    {
        file = dir.absoluteFilePath(file);
    }

    auto iterWatch = watchedFiles.begin();
    auto iterAll = allFiles.begin();
    while (!(iterWatch == watchedFiles.end() && iterAll == allFiles.end()))
    {
        if (iterWatch == watchedFiles.end())
        {
            addWatch(*iterAll);
            ++iterAll;
        }
        else if (iterAll == allFiles.end())
        {
            delWatch(*iterWatch);
            ++iterWatch;
        }
        else if ((*iterAll) < (*iterWatch))
        {
            addWatch(*iterAll);
            ++iterAll;
        }
        else if ((*iterAll) > (*iterWatch))
        {
            delWatch(*iterWatch);
            ++iterWatch;
        }
        else // items match
        {
            ++iterAll;
            ++iterWatch;
        }
    }
    setWindowTitle(path);
}

void MainWindow::slotFileChanged(const QString &path)
{
    QFileInfo fi(path);
    QString name = fi.fileName();

    auto iter = fileInfo.find(name);
    if (fileInfo.end() == iter)
    {
        ui->plainTextEdit->appendPlainText(tr("NOTFOUND: ") + name);
        return;
    }

    // Update the text Output.
    if (fi.exists())
    {
        ui->plainTextEdit->appendPlainText(tr("CHANGED: ") + name);
    }
    else
    {
        delWatch(path);
    }
}

void MainWindow::addWatch(const QString &path)
{
    QFileInfo fi(path);
    QString name = fi.fileName();

    // Update the map.
    fileInfo[name] = fi;

    // Update the text Output.
    if (initialized)
    {
        ui->plainTextEdit->appendPlainText(tr("CREATED: ") + name);
    }

    // Update the widget control
    new QListWidgetItem(name, ui->listWidget);

    // Update the watch
    watcher->addPath(path);
}

void MainWindow::delWatch(const QString &path)
{
    QFileInfo fi(path);
    QString name = fi.fileName();

    // Update the map.
    auto iter = fileInfo.find(name);
    if (fileInfo.end() != iter)
    {
        fileInfo.erase(iter);
    }

    // Update the text Output.
    ui->plainTextEdit->appendPlainText(tr("DELETED: ") + name);

    // Update the widget control
    auto items = ui->listWidget->findItems(name, Qt::MatchExactly);
    for (auto &item : items)
    {
        delete item;
    }

    // Update the watch
    watcher->removePath(path);
}
