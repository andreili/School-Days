#ifndef QSCRIPTACTION_H
#define QSCRIPTACTION_H

#include <QObject>
#include <QTime>
#include <QStringList>

class QScriptAction : public QObject
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
    void setText(QString new_text) { this->text = new_text; }

    QString getPersona() { return this->persona; }
    void setPersona(QString new_persons) { this->persona = new_persons; }

    QString getAnswer1() { return this->answer1; }
    void setAnswer1(QString new_answer) { this->answer1 = new_answer; }
    QString getAnswer2() { return this->answer2; }
    void setAnswer2(QString new_answer) { this->answer2 = new_answer; }

    QString toString();

signals:

public slots:
    void execute();

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
