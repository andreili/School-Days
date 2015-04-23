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

    typedef struct
    {
        Action action;
        QString file_name;
        QString text;
        QString direction;
        QString persona;
        QString answer1, answer2;
        int layer;
        qint64 start;
        qint64 end;

        bool runned;
        bool running;
        int sync_phaze;

        //QSound* sound;
        QActionTimer* sync_timer;
        QImage* image;
        QImage* sync[3];
    } QScriptAction;

private:
    QFileSystem* fs;
    QList<QScriptAction*> actions;
    QList<QScriptAction*> queue_stop;
    QSingleActionTimer* current_timer;
    QString latest_bg_fn;
    qint64 start_time;

    void add_action_by_ors(QString action, QStringList params);
    QString actionToString(QScriptAction* action);
    void checkStopActions();
    void runAction(QScriptAction* action);

signals:
    void SetLayerImage(int layer, QImage* img);

public slots:
    void execute();
    void action_sheduler(void* current_action);
    void action_stop(void* data);
    void action_sync(void* data);

};

#endif // QSCRIPT_H
