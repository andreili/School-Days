#include "qscriptaction.h"
#include <QMetaEnum>
#ifdef QT_DEBUG
#include <QDebug>
#endif

int convert_time(QString time)
{
    QStringList times = time.split(QRegExp("[:;]"), QString::SkipEmptyParts);
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
    case PlayES:
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
    case SkipFRAME:
    case Next:
        return start + ";\n\n";
    case CreateBG:
        body = "\tBGS\t" + this->file_name + "\t";
        break;
    case BlackFade:
    case WhiteFade:
        body = "\t" + this->direction + "\t";
        break;
    case PlayMovie:
        body = "\t" + this->file_name + "\t0\t";
        break;
    case PlaySe:
        body = "\t" + QString::number(this->layer) + "\t" + this->file_name + "\t";
        break;
    case PlayBgm:
    case PlayES:
        body = "\t" + this->file_name + "\t";
        break;
    case PlayVoice:
        if (this->layer == -1)
            body = "\t" + this->file_name + "\t";
        else if (this->persona.length() == 0)
            body = "\t" + this->file_name + "\t" + QString::number(this->layer) + "\t";
        else
            body = "\t" + this->file_name + "\t" + QString::number(this->layer) + "\t" + this->persona + "\t";
        break;
    case PrintText:
        body = "\t" + this->persona + "\t" + this->text + "\t";
        break;
    case SetSELECT:
        body = "\t" + this->answer1 + "\t" + this->answer2 + "\t";
        break;
    case EndBGM:
    case EndRoll:
        body = "\t" + this->file_name + "\t";
        break;
    case MoveSom:
        body = "\t" + QString::number(this->layer) + "\t";
        break;
    }

    return start + body + back_time(this->end) + ";\n\n";
}

void QScriptAction::execute()
{
    switch (this->action)
    {
    case PlaySe:
        this->sound = new QSound(this->file_name);
        this->sound->play();
        this->end_timer = new QTimer();
        this->end_timer->setInterval(this->end - this->start);
        this->end_timer->setSingleShot(true);
        connect(this->end_timer, SIGNAL(timeout()), this, SLOT(stop()));
        this->end_timer->start();
        break;
    case PrintText:
    case SetSELECT:
    case CreateBG:
    case BlackFade:
    case WhiteFade:
    case PlayMovie:
    case PlayBgm:
    case PlayES:
    case PlayVoice:
    case EndBGM:
    case EndRoll:
    case Next:
        break;
    case SkipFRAME:
    case MoveSom:
        break;
    }
}

void QScriptAction::stop()
{
    switch (this->action)
    {
    case PlaySe:
        delete this->sound;
        delete this->end_timer;
        break;
    case PrintText:
    case SetSELECT:
    case CreateBG:
    case BlackFade:
    case WhiteFade:
    case PlayMovie:
    case PlayBgm:
    case PlayES:
    case PlayVoice:
    case EndBGM:
    case EndRoll:
    case Next:
        break;
    case SkipFRAME:
    case MoveSom:
        break;
    }
}
