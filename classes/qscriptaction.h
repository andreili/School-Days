#ifndef QSCRIPTACTION_H
#define QSCRIPTACTION_H

#include <QObject>
#include <QStringList>

class QScriptAction : public QObject
{
    Q_OBJECT
public:

    enum Action
    {
        SkipFRAME,
        CreateBG,
        BlackFade,
        WhiteFade,
        PlayMovie,
        PlaySe,
        PlayBgm,
        PlayVoice,
        PrintText,
        SetSELECT,
        Next,
        EndBGM,
        EndRoll,
        MoveSom
    };

    explicit QScriptAction(Action action, QStringList *params, QObject *parent = 0);

    Action getAction() { return this->action; }
    QString getText() { return this->text; }
    QString getAnswer1() { return this->answer1; }
    QString getAnswer2() { return this->answer2; }

signals:

public slots:

private:
    Action action;
    QString file_name;
    QString text;
    QString direction;
    QString persona;
    QString answer1, answer2;
    int layer;
    int start;
    int end;

};

#endif // QSCRIPTACTION_H
