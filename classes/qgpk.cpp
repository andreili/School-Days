#include "qgpk.h"
#include <QFile>

QGPK::QGPK(QObject *parent) :
    QObject(parent)
{
}

bool QGPK::load(QString file_name)
{
    QFile package(file_name);
    if (!package.open(QFile::ReadOnly))
            return false;
    //package.fileEngine()
    return true;
}
