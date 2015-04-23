#include "qfilesystem.h"
#include <QDir>
#ifdef QT_DEBUG
#include <QDebug>
#endif

QFileSystem::QFileSystem(QString gameRoot, QObject *parent) :
    QObject(parent)
{
    this->root = gameRoot;
    QDir::setCurrent(gameRoot);
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
    if (QFile::exists(this->root + filename))
    {
        return new QGPKFile(this->root + filename, this);
    }

    QString pkg = filename.left(filename.indexOf('/'));
    filename.remove(0, filename.indexOf('/') + 1);
    filename = normalize_name(pkg, filename);
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

void QFileSystem::unpack_all()
{
    foreach (QGPK* gpk, this->gpks)
    {
#ifdef QT_DEBUG
        qDebug() << "Unpacking:" << gpk->getName() << endl;
#endif
        gpk->unpack_all(this->root + gpk->getName() + QDir::separator());
    }
}

void QFileSystem::findArchives()
{
    QString packsRoot = this->root + QDir::separator() + "packs" + QDir::separator();
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
    return normalize_name(name.left(name.indexOf('/')), name);
}

QString QFileSystem::normalize_name(QString pkg, QString name)
{
    if ((pkg.startsWith("SysSe")) || (pkg.startsWith("Se")) || (pkg.startsWith("Voice")))
    {
        return name + ".ogg";
    }
    else if ((pkg.startsWith("BGM")))
    {
        return name + "_loop.ogg";
    }
    else if ((pkg.startsWith("Event")))
    {
        return name + ".PNG";
    }
    else
        return name;
}
