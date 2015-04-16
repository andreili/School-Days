#include "qgpkfile.h"

QGPKFile::QGPKFile(QString realname, QObject *parent) :
    QIODevice(parent)
{
    this->isPKG = false;
    this->realfile.setFileName(realname);
    this->realfile.open(QFile::ReadOnly);
}

QGPKFile::QGPKFile(GPKEntryHeader *entry_header, QString pkg, QObject *parent) :
    QIODevice(parent)
{
    this->isPKG = true;
    this->entry = *entry_header;
    this->posistion = 0;

    this->realfile.setFileName(pkg);
    this->realfile.open(QFile::ReadOnly);
    this->realfile.seek(entry_header->offset);
}

QGPKFile::~QGPKFile()
{
}

qint64 QGPKFile::read(char *data, qint64 maxlen)
{
    if (this->isPKG)
    {
        quint64 res = this->realfile.read(data, maxlen);
        this->posistion += res;
        return res;
    }
    else
    {
        return this->realfile.read(data, maxlen);
    }
}

bool QGPKFile::seek(qint64 offset)
{
    if (this->isPKG)
        this->posistion = offset;
    else
        return this->realfile.seek(offset);
    return true;
}

void QGPKFile::close()
{
    if (this->isPKG)
    {
        this->realfile.close();
    }
    else
        this->realfile.close();
}

qint64 QGPKFile::readData(char *data, qint64 maxlen)
{
    return 0;
}

qint64 QGPKFile::writeData(const char *data, qint64 maxlen)
{
    return 0;
}
