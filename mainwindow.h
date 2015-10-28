#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>

#include <map>
#include <memory>

class QFileSystemWatcher;

namespace Ui {
class MainWindow;
}

struct FileInfoAndData
{
    QFileInfo info;
    std::unique_ptr<char[]> data;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotCloseWatch();
    void slotOpenWatch();
    void slotDirectoryChanged(const QString& path);
    void slotFileChanged(const QString& path);

private:

    void addWatch(const QString& path);
    void delWatch(const QString& path);

    Ui::MainWindow *ui;
    QFileSystemWatcher *watcher;

    typedef std::map<QString, FileInfoAndData> map_t;
    map_t fileInfo;
    bool  initialized;
};

#endif // MAINWINDOW_H
