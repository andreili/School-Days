#include "qgpk.h"
#include <QFile>
#include <QDir>
#include "qgpkfile.h"
#ifdef QT_DEBUG
#include <QDebug>
#endif

QGPK::QGPK(QObject *parent) :
    QObject(parent)
{
}

bool QGPK::load(QString file_name)
{
    this->name = file_name;

    QFile package(file_name);
    if (!package.open(QFile::ReadOnly))
            return false;

    GPKsig sign;
    package.seek(package.size() - sizeof(GPKsig));
    if ((package.read((char*)&sign, sizeof(GPKsig)) != sizeof(GPKsig)) ||
            (strncmp(GPK_TAILER_IDENT0, sign.sig0, strlen(GPK_TAILER_IDENT0)) != 0) ||
            (strncmp(GPK_TAILER_IDENT1, sign.sig1, strlen(GPK_TAILER_IDENT1)) != 0))
    {
#ifdef QT_DEBUG
        qDebug() << "GPK: broken sign";
#endif
        return false;
    }

    package.seek(package.size() - sizeof(GPKsig) - sign.pidx_length);
    QByteArray compressedData = package.read(sign.pidx_length);

    int k = 0;
    for (int i=0 ; i<compressedData.length() ; i++)
    {
        compressedData[i] = compressedData[i] ^ CIPHERCODE[k++];
        if (k > 15)
            k = 0;
    }

    // qUncompress used BigEndian size!
    compressedData[3] = compressedData[0];
    compressedData[2] = compressedData[1];
    compressedData[1] = 0;
    compressedData[0] = 0;
    QByteArray uncompressedData = qUncompress(compressedData);

    QDataStream uncompressedStream(uncompressedData);
    uncompressedStream.setByteOrder(QDataStream::LittleEndian);
    while (!uncompressedStream.atEnd())
    {
        GPKentry entry;

        quint16 filename_len;
        uncompressedStream >> filename_len;
        QByteArray buf(filename_len * 2, Qt::Uninitialized);
        uncompressedStream.readRawData(buf.data(), filename_len*2);
        buf.append('\0');
        buf.append('\0');
        entry.name = QString::fromUtf16((ushort*)buf.constData());

        uncompressedStream.readRawData((char*)&entry.header, sizeof(GPKEntryHeader));

        this->entries.append(entry);
    }

    package.close();
    return true;
}

QGPKFile *QGPK::open(QString filename)
{
    foreach (GPKentry entry, this->entries)
    {
        if (entry.name.compare(filename) == 0)
            return new QGPKFile(&entry.header, this->name, this);
    }
    return NULL;
}

QStringList QGPK::list(QString mask)
{
    QStringList res;
    foreach (const GPKentry &entry, this->entries)
    {
        if ((name.length() > 0) && (QRegExp(mask, Qt::CaseInsensitive).exactMatch(entry.name)))
            res.append(entry.name);
    }
    return res;
}

QString QGPK::getName()
{
    return this->name.right(this->name.length() - this->name.lastIndexOf(QDir::separator()) - 1).replace(".GPK", "");
}
