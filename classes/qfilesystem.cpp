#include "qfilesystem.h"
#include <QDir>
#ifdef QT_DEBUG
#include <QDebug>
#endif

QFileSystem::QFileSystem(QString gameRoot, QObject *parent) :
    QObject(parent)
{
    this->root = gameRoot;
    findArchives();
}

QFileSystem::~QFileSystem()
{
    foreach (QGPK* gpk, this->gpks)
    {
        delete gpk;
    }
}

QGPKFile* QFileSystem::open(QString filename)
{
    if (QFile::exists(filename))
    {
        return new QGPKFile(filename);
    }

    QString pkg = filename.left(filename.indexOf(QDir::separator()));
    filename.remove(0, filename.indexOf(QDir::separator()) + 1);
    filename = normalize_name(filename);
    foreach (QGPK* gpk, this->gpks)
    {
        if (gpk->getName().compare(pkg, Qt::CaseInsensitive) == 0)
        {
            QGPKFile* res = gpk->open(filename);
            if (res)
                return res;
        }
    }
    return NULL;
}

QStringList QFileSystem::list(QString mask)
{
    QStringList res;
    QString pkg = mask.left(mask.indexOf(QDir::separator()));
    mask.remove(0, mask.indexOf(QDir::separator()) + 1);
    foreach (QGPK* gpk, this->gpks)
    {
        if (gpk->getName().compare(pkg, Qt::CaseInsensitive) == 0)
            res.append(gpk->list(mask));
    }
    return res;
}

void QFileSystem::findArchives()
{
    QString packsRoot = this->root + QDir::separator() + "packs";
    QDir packsDir(packsRoot, "*.GPK");
    QStringList packs = packsDir.entryList();
    foreach (QString pack_name, packs) {
        mountGPK(packsRoot + QDir::separator() + pack_name);
#ifdef QT_DEBUG
        qDebug() << "Mounted package:" << pack_name;
#endif
    }
}

void QFileSystem::mountGPK(QString fileName)
{
    QGPK* gpk = new QGPK();
    if (gpk->load(fileName))
    {
        this->gpks.append(gpk);
    }
}

QString QFileSystem::normalize_name(QString name)
{
    if ((name.startsWith("BGM")) || (name.startsWith("SysSe")))
    {
        return name + ".ogg";
    }
    else
        return name;
}
