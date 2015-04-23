#ifndef QFILESYSTEM_H
#define QFILESYSTEM_H

#include <QObject>
#include <QList>
#include "qgpk.h"
#include "qgpkfile.h"

class QFileSystem : public QObject
{
    Q_OBJECT
public:
    explicit QFileSystem(QString gameRoot, QObject *parent = 0);
    ~QFileSystem();

    QGPKFile *open(QString filename);

    QStringList list(QString mask);

    QString getRoot() { return this->root; }

    void unpack_all();

    QString normalize_name(QString name);

signals:

public slots:

private:
    QString root;
    QList<QGPK*> gpks;

    void findArchives();
    void mountGPK(QString fileName);

    inline QString normalize_name(QString pkg, QString name);
};

#endif // QFILESYSTEM_H
