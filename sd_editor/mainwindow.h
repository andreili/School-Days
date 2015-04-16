#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../classes/qfilesystem.h"
#include "../classes/qscript.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public Q_SLOTS:
    void SetGameFolder();

private slots:
    void on_twScripts_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    QFileSystem* fs;
    QScript* activeScript;

};

#endif // MAINWINDOW_H
