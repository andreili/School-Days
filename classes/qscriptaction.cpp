#include "qscriptaction.h"
#include <QMetaEnum>
#ifdef QT_DEBUG
#include <QDebug>
#endif

int convert_time(QString time)
{
    qDebug() << time;
    QStringList times = time.split(':');
    return (times[0].toInt() * 60 +         // minutes
            times[1].toInt()) * 1000 +      //seconds
           (times[2].toFloat() * 16.6667);  // milliseconds
}

QString back_time(int msecs)
{
    float ms = msecs % 1000;
    msecs -= ms;
    msecs /= 1000;
    int secs = msecs % 60;
    int min = (msecs - secs) / 60;
    return QString("%1").arg(min, 2, 10, QChar('0')) + ":" +
            QString("%1").arg(secs, 2, 10, QChar('0')) + ":" +
            QString("%1").arg(qRound(ms / 16.6667), 2, 10, QChar('0'));
}

QScriptAction::QScriptAction(Action action, QStringList *params, QObject *parent) :
    QObject(parent)
{
    this->layer = -1;
    this->end = -1;

    this->action = action;
    this->start = convert_time(params->at(0).split('=').at(1));

    switch (action)
    {
    case SkipFRAME:
    case Next:
        break;
    case CreateBG:
        this->file_name = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case BlackFade:
    case WhiteFade:
        this->direction = params->at(1);
        this->end = convert_time(params->at(2));
        break;
    case PlayMovie:
        this->file_name = params->at(1);
        this->end = convert_time(params->at(3));
        break;
    case PlaySe:
        this->layer = params->at(1).toInt();
        this->file_name = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case PlayBgm:
        this->file_name = params->at(1);
        this->end = convert_time(params->at(2));
        break;
    case PlayVoice:
        this->file_name = params->at(1);
        if (params->count() == 3)
            this->end = convert_time(params->at(2));
        else if (params->count() == 4)
        {
            this->layer = params->at(2).toInt();
            this->end = convert_time(params->at(3));
        }
        else
        {
            this->layer = params->at(2).toInt();
            this->persona = params->at(3);
            this->end = convert_time(params->at(4));
        }
        break;
    case PrintText:
        this->persona = params->at(1);
        this->text = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case SetSELECT:
        this->answer1 = params->at(1);
        this->answer2 = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case EndBGM:
    case EndRoll:
        this->file_name = params->at(1);
        this->end = convert_time(params->at(2));
        break;
    case MoveSom:
        this->layer = params->at(1).toInt();
        this->end = convert_time(params->at(2));
        break;
    default:
#ifdef QT_DEBUG
        QMetaEnum en = QScriptAction::staticMetaObject.enumerator(0);
        qDebug() << "Unknown action: " << en.valueToKey(action);
#endif
        break;
    }
}

QString QScriptAction::toString()
{
    QMetaEnum en = QScriptAction::staticMetaObject.enumerator(0);
    QString start = "[" + QString(en.valueToKey(this->action)) +
            "]=" + back_time(this->start);

    QString body = "";
    switch (this->action)
    {
    case QScriptAction::SkipFRAME:
    case QScriptAction::Next:
        return start + ";\n\n";
    case QScriptAction::CreateBG:
        body = "\tBGS\t" + this->file_name + "\t";
        break;
    case QScriptAction::BlackFade:
    case QScriptAction::WhiteFade:
        body = "\t" + this->direction + "\t";
        break;
    case QScriptAction::PlayMovie:
        body = "\t" + this->file_name + "\t0\t";
        break;
    case QScriptAction::PlaySe:
        body = "\t" + QString::number(this->layer) + "\t" + this->file_name + "\t";
        break;
    case QScriptAction::PlayBgm:
        body = "\t" + this->file_name + "\t";
        break;
    case QScriptAction::PlayVoice:
        if (this->layer == -1)
            body = "\t" + this->file_name + "\t";
        else if (this->persona.length() == 0)
            body = "\t" + this->file_name + "\t" + QString::number(this->layer) + "\t";
        else
            body = "\t" + this->file_name + "\t" + QString::number(this->layer) + "\t" + this->persona + "\t";
        break;
    case QScriptAction::PrintText:
        body = "\t" + this->persona + "\t" + this->text + "\t";
        break;
    case QScriptAction::SetSELECT:
        body = "\t" + this->answer1 + "\t" + this->answer2 + "\t";
        break;
    case QScriptAction::EndBGM:
    case QScriptAction::EndRoll:
        body = "\t" + this->file_name + "\t";
        break;
    case QScriptAction::MoveSom:
        body = "\t" + QString::number(this->layer) + "\t";
        break;
    }

    return start + body + back_time(this->end) + ";\n\n";
}

void QScriptAction::execute()
{
    // TODO
}
