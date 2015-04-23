#ifndef QSCRIPT_H
#define QSCRIPT_H

#include <QList>
#include <QImage>
#include <QObject>
#include <QStringList>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAbstractVideoSurface>
#include "qgpkfile.h"
#include "qfilesystem.h"
#include "qactiontimer.h"

class QScript : public QAbstractVideoSurface
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

        QMediaPlayer* player;
        QActionTimer* sync_timer;
        QImage* image;
        QImage* prev_frame;
        QImage* sync[3];
    } QScriptAction;

    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(
                QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

private:
    QFileSystem* fs;
    QList<QScriptAction*> actions;
    QList<QScriptAction*> queue_stop;
    QScriptAction* playing_video;
    QSingleActionTimer* current_timer;
    QString latest_bg_fn;
    qint64 start_time;

    void add_action_by_ors(QString action, QStringList params);
    QString actionToString(QScriptAction* action);
    void checkStopActions();
    bool runAction(QScriptAction* action);

signals:
    void SetLayerImage(int layer, QImage* img);
    void repaintCanvas();

public slots:
    void execute();
    void action_sheduler(void* current_action);
    void action_stop(void* data);
    void action_sync(void* data);

private slots:
    virtual bool present(const QVideoFrame &frame);
};

#endif // QSCRIPT_H
