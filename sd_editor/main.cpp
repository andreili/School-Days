//#include "mainwindow.h"
//#include <QApplication>
#include <QTextCodec>

#include <QDebug>
#include <QFile>
#include <QDir>
#include "../classes/qscript.h"

int main(int argc, char *argv[])
{
    QTextCodec *utfCodec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(utfCodec);

    if (argc < 1)
        return 1;

    QString path = "/media/work/Dev/Games/SD/_/Script/ENGLISH";
    //QString path = argv[1];
    QDir dirs(path);
    dirs.setFilter(QDir::Dirs);
    QStringList folders = dirs.entryList();
    foreach (QString folder, folders)
    {
        if (folder[0] == '.')
            continue;

        QDir files_dir(path + "/" + folder);
        files_dir.setNameFilters(QStringList() << "*.ENG.ORS");
        files_dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        QStringList files = files_dir.entryList();
        foreach (QString file, files)
        {
            qDebug() << file;
            QScript* script = new QScript();
            QFile scr(path + "/" + folder + "/" + file);
            if (scr.open(QFile::ReadOnly))
            {
                qDebug() << "open";
                script->load_from_ORS(&scr);
                qDebug() << "export";
                script->export_txt(path + "/" + folder + "/" + file + ".txt");
                qDebug() << "import";
                script->import_txt(path + "/" + folder + "/" + file + ".txt");
            }
            else
                qDebug() << scr.errorString();
            delete script;
        }
    }

    return 0;

    /*QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();*/
}
