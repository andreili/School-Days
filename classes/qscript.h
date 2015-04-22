#ifndef QSCRIPT_H
#define QSCRIPT_H

#include <QList>
#include <QTimer>
#include <QObject>
#include <QStringList>
#include "qscriptaction.h"
#include "qgpkfile.h"

class QScript : public QObject
{
    Q_OBJECT
public:
    explicit QScript(QObject *parent = 0);
    ~QScript();

    void load_from_ORS(QGPKFile *file);

    QString serialize();

    void export_txt(QString file_name);
    void import_txt(QString file_name);

private:
    QList<QScriptAction*> actions;
    QList<QTimer*> timers;

    void add_action_by_ors(QString action, QStringList params);

signals:

public slots:

};

#endif // QSCRIPT_H
