#ifndef QFILESYSTEM_H
#define QFILESYSTEM_H

#include <QObject>

class QFileSystem : public QObject
{
    Q_OBJECT
public:
    explicit QFileSystem(QObject *parent = 0);

    void init(QString root_dir = "packs/");

signals:

public slots:

};

#endif // QFILESYSTEM_H
