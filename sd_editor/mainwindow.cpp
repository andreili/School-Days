#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDir>
//#include <QtMultimediaWidgets/QVideoWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

   /* QMediaPlayer *player = new QMediaPlayer();
   // QFile * str = new QFile("h:\\Music\\nya_nyako_nya.mp3");
    //str->open(QFile::ReadOnly);
    //player->setMedia(QMediaContent(), str);
    QVideoWidget *videoWidget = new QVideoWidget;
    player->setVideoOutput(videoWidget);
    this->setCentralWidget(videoWidget);
    videoWidget->show();

    player->setMedia(QUrl::fromLocalFile("h:\\1.ogg"));
    player->setVolume(255);
    player->play();
    qDebug() << player->errorString();*/

    connect(ui->mSetGameFolder, SIGNAL(triggered()), this, SLOT(SetGameFolder()));

    engine_widget = new EngineWidget(this);
    ui->wEngine->layout()->addWidget(engine_widget);

    this->activeScript = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetGameFolder()
{
    //QString game_dir = QFileDialog::getExistingDirectory(this, "Укажите папку с игрой");
    QString game_dir = "/media/work/Dev/Games/SD";
    fs = new QFileSystem(game_dir, this);

    QStringList scripts = fs->list(QString("Script") + QDir::separator() + ".*");
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
    if (ui->twScripts->currentItem()->parent() != NULL)
    {
        QString fn = "Script/ENGLISH/" + ui->twScripts->currentItem()->parent()->text(0) + "/" +
                ui->twScripts->currentItem()->text(0);

        if (this->activeScript != NULL)
            delete this->activeScript;

        this->activeScript = new QScript(fs);
        this->activeScript->load_from_ORS(fs->open(fn));
        //this->activeScript->load_from_ORS(new QGPKFile(fn));
        connect(this->activeScript, SIGNAL(SetLayerImage(int,QImage*)), this->engine_widget,
                SLOT(SetLayerImage(int,QImage*)));
        this->activeScript->execute();
       /* QMediaPlayer *player = new QMediaPlayer();
        //player->setMedia(QMediaContent(), fs->open("System/OP/SDHQ_OP"));
        player->setMedia(QMediaContent(), fs->open("Se00/00-00/00-00-A02/SE00-00-A02-001"));
        player->setVolume(50);
        player->play();*/
    }
}
