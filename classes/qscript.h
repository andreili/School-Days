#ifndef QSCRIPT_H
#define QSCRIPT_H

#include <QList>
#include <QImage>
#include <QObject>
#include <QStringList>
#include "qgpkfile.h"
#include "qfilesystem.h"
#include "qactiontimer.h"

class QScript : public QObject
{
    Q_OBJECT
    Q_ENUMS(Action)
public:
    enum Action
    {
        SkipFRAME,
        CreateBG,
        BlackFade,
        WhiteFade,
        PlayMovie,
        PlaySe,
        PlayES,
        PlayBgm,
        PlayVoice,
        PrintText,
        SetSELECT,
        Next,
        EndBGM,
        EndRoll,
        MoveSom
    };

    explicit QScript(QFileSystem* fs, QObject *parent = 0);
    ~QScript();

    void load_from_ORS(QGPKFile *file);

    QString serialize();

    void export_txt(QString file_name);
    void import_txt(QString file_name);

private:
    typedef struct
    {
        Action action;
        QString file_name;
        QString text;
        QString direction;
        QString persona;
        QString answer1, answer2;
        int layer;
        int start;
        int end;

        bool runned;

        //QSound* sound;
        QActionTimer* end_timer;
        QImage* image;
    } QScriptAction;

    QFileSystem* fs;
    QList<QScriptAction*> actions;
    QList<QActionTimer*> timers;

    void add_action_by_ors(QString action, QStringList params);
    QString actionToString(QScriptAction* action);

signals:
    void SetLayerImage(int layer, QImage* img);

public slots:
    void execute();
    void action_execute(void* data);
    void action_stop(void* data);

};

#endif // QSCRIPT_H
