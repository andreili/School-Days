#ifndef QGPKFILE_H
#define QGPKFILE_H

#include <QtCore/qiodevice.h>

class QGPKFile : public QIODevice
{
    Q_OBJECT
public:
    explicit QGPKFile(QObject *parent = 0);

signals:

public slots:

};

#endif // QGPKFILE_H
