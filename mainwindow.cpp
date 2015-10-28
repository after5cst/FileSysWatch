#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fcntl.h>
#include <unistd.h>

#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QListWidgetItem>

#define FILE_CACHE_SIZE_LIMIT (1024 * 1024 * 10)
static_assert(FILE_CACHE_SIZE_LIMIT < SSIZE_MAX,
              "File cache size limit is too large");

namespace
{
FileInfoAndData loadReasonableTextFile(const QString &path)
{
    FileInfoAndData out;
    out.info = QFileInfo(path);
    if (!((out.info.exists()) && (out.info.isFile()) &&
          (out.info.size() < FILE_CACHE_SIZE_LIMIT) &&
          (out.info.isExecutable() == false) && (out.info.isReadable())))
    {
        return out;
    }
    std::unique_ptr<char[]> p(new char[out.info.size()]);

    auto fh = open(path.toUtf8().constData(), O_RDONLY);
    if (-1 == fh)
    {
        return out;
    }

    auto bytesRead = read(fh, p.get(), out.info.size());
    close(fh);

    if (bytesRead != out.info.size())
    {
        return out;
    }

    // If it's not printable, we don't want the file.
    for (const char *q = p.get(); bytesRead--;)
    {
        if (!isprint(*q))
        {
            return out;
        }
    }

    out.data = std::move(p);
    return out;
}
}

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
    auto curr = loadReasonableTextFile(path);
    QString name = curr.info.fileName();
    auto &prev = fileInfo[name];
    QString changeType;

    if (curr.info.exists())
    {
        changeType = (prev.info.exists() ? "MOD" : "ADD");
    }
    else
    {
        changeType = (prev.info.exists() ? "DEL" : "UNK");
    }

    auto newSize = curr.info.size();
    auto oldSize = prev.info.size();

    if (newSize == oldSize)
    {
        int compare = memcmp(curr.data.get(), prev.data.get(), oldSize);
        if (0 != compare)
        {
            ui->plainTextEdit->appendPlainText(
                QString(tr("%1[%2]: Data modified in place."))
                    .arg(changeType, name));
        }
        else if ("MOD" != changeType)
        {
            ui->plainTextEdit->appendPlainText(
                QString(tr("%1[%2]: Empty file.")).arg(changeType, name));
        }
    }
    else
    {
        auto strOldSize = QString::number(oldSize);
        auto strNewSize = QString::number(newSize);
        ui->plainTextEdit->appendPlainText(
            QString(tr("%1[%2]: File size changed from %3 to %4."))
                .arg(changeType, name, strOldSize, strNewSize));

        if (newSize > oldSize)
        {
            int compare = memcmp(curr.data.get(), prev.data.get(), oldSize);
            if (0 == compare)
            {
                auto newBytes = newSize - oldSize;
                const char *p = curr.data.get() + oldSize;
                ui->plainTextEdit->appendPlainText(
                    QString::fromUtf8(p, newBytes));
            }
        }
    }

    // Save off the now-known state of the file.
    prev = std::move(curr);

    // Wait until last to remove our cache contents.
    if (!prev.info.exists())
    {
        auto iter = fileInfo.find(name);
        if (fileInfo.end() != iter)
        {
            fileInfo.erase(iter);
        }

        // Update the widget control
        auto items = ui->listWidget->findItems(name, Qt::MatchExactly);
        for (auto &item : items)
        {
            delete item;
        }

        // Update the watch
        watcher->removePath(path);
    }
}

void MainWindow::addWatch(const QString &path)
{
    auto fiad = loadReasonableTextFile(path);
    QString name = fiad.info.fileName();

    // Update the text Output.
    if (initialized)
    {
        slotFileChanged(path);
    }

    // Update the widget control
    new QListWidgetItem(name, ui->listWidget);

    // Update the watch
    watcher->addPath(path);
}

void MainWindow::delWatch(const QString &path) { slotFileChanged(path); }
