#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QFileSystemWatcher;

namespace Ui {
class MainWindow;
}

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
};

#endif // MAINWINDOW_H
