#ifndef QGPKFILE_H
#define QGPKFILE_H

#include <QtCore/qiodevice.h>
#include <QFile>
#include "qgpk.h"

class QGPKFile : public QObject
{
    Q_OBJECT
public:
    explicit QGPKFile(QString realname, QObject  *parent = 0);
    explicit QGPKFile(GPKEntryHeader* entry_header, QString pkg, QObject  *parent = 0);
    ~QGPKFile();

    qint64 read(char *data, qint64 maxlen);
    qint64 pos() { return this->posistion; }
    bool atEnd() { return (this->isPKG) ? (this->posistion >= entry.comprlen) : (this->realfile.atEnd()); }
    bool seek(qint64 offset);
    qint64 size() { return this->entry.comprlen; }
    void close();
    QByteArray readLine(qint64 maxlen = 0);

signals:

public slots:

private:
    QFile realfile;
    bool isPKG;
    bool compressed;
    GPKEntryHeader entry;
    qint64 posistion;


    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
};

#endif // QGPKFILE_H
