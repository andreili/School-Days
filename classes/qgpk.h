#ifndef QGPK_H
#define QGPK_H

#include <QObject>

class QGPK : public QObject
{
    Q_OBJECT
public:
    explicit QGPK(QObject *parent = 0);

    bool load(QString file_name);

signals:

public slots:

};

#endif // QGPK_H
