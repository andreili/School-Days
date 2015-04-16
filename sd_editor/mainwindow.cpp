#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->mSetGameFolder, SIGNAL(triggered()), this, SLOT(SetGameFolder()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetGameFolder()
{
    QString game_dir = QFileDialog::getExistingDirectory(this, "Укажите папку с игрой");
    fs = new QFileSystem(game_dir, this);

    QStringList scripts = fs->list("Script/.*");
    QTreeWidgetItem* top = NULL;
    ui->twScripts->clear();
    foreach (QString name, scripts)
    {
        name.remove(0, name.indexOf('/') + 1);
        QString top_name = name.left(2);
        name.remove(0, 3);
        if ((!top) || (top->text(0).compare(top_name) != 0))
        {
            top = new QTreeWidgetItem();
            top->setText(0, top_name);
            ui->twScripts->addTopLevelItem(top);
        }
        QTreeWidgetItem* node = new QTreeWidgetItem();
        node->setText(0, name);
        top->addChild(node);
    }
}

void MainWindow::on_twScripts_doubleClicked(const QModelIndex &index)
{
    QString fn = "Script/ENGLISH/" + ui->twScripts->currentItem()->parent()->text(0) + "/" +
            ui->twScripts->currentItem()->text(0);
    this->activeScript = new QScript();
    this->activeScript->load_from_ORS(fs->open(fn));    // TODO
}
