#include "qscript.h"
#include <QTextStream>
#include <QFile>
#include <QMetaEnum>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

int convert_time(QString time)
{
    QStringList times = time.split(QRegExp("[:;]"), QString::SkipEmptyParts);
    return (times[0].toInt() * 60 +         // minutes
            times[1].toInt()) * 1000 +      // seconds
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

QScript::QScript(QFileSystem* fs, QObject *parent) :
    QObject(parent)
{
    this->fs = fs;
}

QScript::~QScript()
{
    foreach(QScriptAction* action, actions)
    {
        delete action;
    }
    foreach(QActionTimer* timer, timers)
    {
        delete timer;
    }
}

void QScript::load_from_ORS(QGPKFile *file)
{
    while (!file->atEnd())
    {
        QString line = file->readLine();
        if ((line.length() == 0) || (line[0] != '['))
            continue;

        this->add_action_by_ors(line.split('\t')[0].split(QRegExp("[:=\\[\\];]"), QString::SkipEmptyParts)[0],
                line.split('\t', QString::SkipEmptyParts));
    }
}

QString QScript::serialize()
{
    QJsonArray obj;
    int idx = 0;
    foreach(QScriptAction* action, actions)
    {
        QJsonObject action_obj;

        QMetaEnum en = staticMetaObject.enumerator(0);
        action_obj["action"] = QString(en.valueToKey(action->action));
        action_obj["start"] = action->start;

        switch (action->action)
        {
        case SkipFRAME:
        case Next:
            obj[idx] = action_obj;
            obj.append(action_obj);
            idx++;
            continue;
        case PrintText:
            action_obj["text"] = action->text;
            action_obj["persona"] = action->persona;
            break;
        case SetSELECT:
            action_obj["answer1"] = action->answer1;
            action_obj["answer2"] = action->answer2;
            break;
        case PlaySe:
            action_obj["layer"] = action->layer;
        case CreateBG:
        case PlayMovie:
        case PlayBgm:
        case PlayES:
        case EndBGM:
        case EndRoll:
            action_obj["file"] = action->file_name;
            break;
        case BlackFade:
        case WhiteFade:
            action_obj["dir"] = action->direction;
            break;
        case PlayVoice:
            action_obj["file"] = action->file_name;
            action_obj["layer"] = action->layer;
            action_obj["persona"] = action->persona;
            break;
        case MoveSom:
            break;
        }

        action_obj["end"] = action->end;
        obj.append(action_obj);
        idx++;
    }

    QJsonDocument doc(obj);
    return QString(doc.toJson());
}

void QScript::export_txt(QString file_name)
{
    QFile text(file_name);
    QTextStream text_stream(&text);

    if (!text.open(QFile::WriteOnly))
        return;

    foreach(QScriptAction* action, actions)
    {
        switch (action->action)
        {
        case PrintText:
            text_stream << action->persona << endl;
            text_stream << action->text << endl;
            break;
        case SetSELECT:
            text_stream << action->answer1 << endl;
            text_stream << action->answer2 << endl;
            break;
        case SkipFRAME:
        case CreateBG:
        case BlackFade:
        case WhiteFade:
        case PlayMovie:
        case PlayBgm:
        case PlaySe:
        case PlayES:
        case PlayVoice:
        case Next:
        case EndBGM:
        case EndRoll:
        case MoveSom:
            break;
        }
    }

    text_stream.flush();
    text.close();
}

void QScript::import_txt(QString file_name)
{
    QFile text(file_name);
    QTextStream text_stream(&text);
    if (!text.open(QFile::ReadOnly))
        return;

    QFile ors(file_name.replace(".txt", "", Qt::CaseInsensitive) + ".ENG.ORS");
    QTextStream ors_stream(&ors);
    if (!ors.open(QFile::WriteOnly))
        return;

    foreach(QScriptAction* action, actions)
    {
        switch (action->action)
        {
        case PrintText:
            action->persona = text_stream.readLine();
            action->text = text_stream.readLine();
            break;
        case SetSELECT:
            action->answer1 = text_stream.readLine();
            action->answer2 = text_stream.readLine();
            break;
        case SkipFRAME:
        case CreateBG:
        case BlackFade:
        case WhiteFade:
        case PlayMovie:
        case PlayBgm:
        case PlaySe:
        case PlayES:
        case PlayVoice:
        case Next:
        case EndBGM:
        case EndRoll:
        case MoveSom:
            break;
        }

        ors_stream << actionToString(action);
    }

    text.close();
    ors_stream.flush();
    ors.close();
}

void QScript::execute()
{
    // TODO: preload data
    foreach(QScriptAction* action, actions)
    {
        switch (action->action)
        {
        case PlaySe:
        case PlayBgm:
        case PlayES:
        case PlayVoice:
        case EndBGM:
        case CreateBG:
            this->timers.append(new QActionTimer(action, action->start, this, SLOT(action_execute(void*))));
            //QTimer::singleShot(action.start, this, SLOT(action_execute()));
            /*timer = new QTimer(this);
            timer->setInterval(action.start);
            timer->setSingleShot(true);
            //connect(timer, SIGNAL(timeout()), action, SLOT(execute()));
            timer->start();
            this->timers.append(timer);*/
            break;
        case PrintText:
        case SetSELECT:
        case BlackFade:
        case WhiteFade:
        case PlayMovie:
        case EndRoll:
            break;
        case SkipFRAME:
        case Next:
        case MoveSom:
            break;
        }
    }
}

void QScript::action_execute(void* data)
{
    QScriptAction* action = (QScriptAction*)data;
#ifdef QT_DEBUG
    QMetaEnum en = staticMetaObject.enumerator(0);
    qDebug() << "Start action" << en.valueToKey(action->action) << " at " << action->start;
#endif
    switch (action->action)
    {
    case PlayBgm:
    case PlayES:
    case PlayVoice:
    case PlaySe:
    case EndBGM:
        //action->sound = new QSound(fs->getPackDir() + fs->normalize_name(thisactionfile_name));
        //action->sound->play();
        /*action->end_timer = new QTimer();
        action->end_timer->setInterval(action->end - action->start);
        action->end_timer->setSingleShot(true);
        connect(action->end_timer, SIGNAL(timeout()), this, SLOT(stop()));
        action->end_timer->start();*/
        break;
    case CreateBG:
        action->image = new QImage(fs->getPackDir() + fs->normalize_name(action->file_name));
        emit SetLayerImage(0, action->image);
        break;
    case PrintText:
    case SetSELECT:
    case BlackFade:
    case WhiteFade:
    case PlayMovie:
    case EndRoll:
    case Next:
        break;
    case SkipFRAME:
    case MoveSom:
        break;
    }
    new QActionTimer(data, action->end - action->start, this, SLOT(action_stop(void*)));
}

void QScript::action_stop(void* data)
{
    QScriptAction* action = (QScriptAction*)data;
#ifdef QT_DEBUG
    QMetaEnum en = staticMetaObject.enumerator(0);
    qDebug() << "Stop action" << en.valueToKey(action->action) << " at " << action->end;
#endif
    switch (action->action)
    {
    case PlayBgm:
    case PlayES:
    case PlayVoice:
    case PlaySe:
    case EndBGM:
        //delete action->sound;
        //delete action->end_timer;
        break;
    case CreateBG:
        emit SetLayerImage(0, NULL);
        delete action->image;
        break;
    case PrintText:
    case SetSELECT:
    case BlackFade:
    case WhiteFade:
    case PlayMovie:
    case EndRoll:
    case Next:
        break;
    case SkipFRAME:
    case MoveSom:
        break;
    }
}

void QScript::add_action_by_ors(QString action, QStringList params)
{
    QMetaEnum en = staticMetaObject.enumerator(0);
    /*actions.append(new QScriptAction((Action)en.keyToValue(action.toStdString().c_str()),
                                     &params, fs, this));*/
    QScriptAction* action_rec = new QScriptAction;

    action_rec->layer = -1;
    action_rec->end = -1;

    action_rec->action = (Action)en.keyToValue(action.toStdString().c_str());
    action_rec->start = convert_time(params.at(0).split('=').at(1));

    switch (action_rec->action)
    {
    case SkipFRAME:
    case Next:
        break;
    case CreateBG:
        action_rec->file_name = params.at(2);
        action_rec->end = convert_time(params.at(3));
        break;
    case BlackFade:
    case WhiteFade:
        action_rec->direction = params.at(1);
        action_rec->end = convert_time(params.at(2));
        break;
    case PlayMovie:
        action_rec->file_name = params.at(1);
        action_rec->end = convert_time(params.at(3));
        break;
    case PlaySe:
        action_rec->layer = params.at(1).toInt();
        action_rec->file_name = params.at(2);
        action_rec->end = convert_time(params.at(3));
        break;
    case PlayBgm:
    case PlayES:
        action_rec->file_name = params.at(1);
        action_rec->end = convert_time(params.at(2));
        break;
    case PlayVoice:
        action_rec->file_name = params.at(1);
        if (params.count() == 3)
            action_rec->end = convert_time(params.at(2));
        else if (params.count() == 4)
        {
            action_rec->layer = params.at(2).toInt();
            action_rec->end = convert_time(params.at(3));
        }
        else
        {
            action_rec->layer = params.at(2).toInt();
            action_rec->persona = params.at(3);
            action_rec->end = convert_time(params.at(4));
        }
        break;
    case PrintText:
        action_rec->persona = params.at(1);
        action_rec->text = params.at(2);
        action_rec->end = convert_time(params.at(3));
        break;
    case SetSELECT:
        action_rec->answer1 = params.at(1);
        action_rec->answer2 = params.at(2);
        action_rec->end = convert_time(params.at(3));
        break;
    case EndBGM:
    case EndRoll:
        action_rec->file_name = params.at(1);
        action_rec->end = convert_time(params.at(2));
        break;
    case MoveSom:
        action_rec->layer = params.at(1).toInt();
        action_rec->end = convert_time(params.at(2));
        break;
    default:
#ifdef QT_DEBUG
        qDebug() << "Unknown action: " << action;
#endif
        break;
    }

    actions.append(action_rec);
}

QString QScript::actionToString(QScriptAction* action)
{
    QMetaEnum en = staticMetaObject.enumerator(0);
    QString start = "[" + QString(en.valueToKey(action->action)) +
            "]=" + back_time(action->start);

    QString body = "";
    switch (action->action)
    {
    case SkipFRAME:
    case Next:
        return action->start + ";\n\n";
    case CreateBG:
        body = "\tBGS\t" + action->file_name + "\t";
        break;
    case BlackFade:
    case WhiteFade:
        body = "\t" + action->direction + "\t";
        break;
    case PlayMovie:
        body = "\t" + action->file_name + "\t0\t";
        break;
    case PlaySe:
        body = "\t" + QString::number(action->layer) + "\t" + action->file_name + "\t";
        break;
    case PlayBgm:
    case PlayES:
        body = "\t" + action->file_name + "\t";
        break;
    case PlayVoice:
        if (action->layer == -1)
            body = "\t" + action->file_name + "\t";
        else if (action->persona.length() == 0)
            body = "\t" + action->file_name + "\t" + QString::number(action->layer) + "\t";
        else
            body = "\t" + action->file_name + "\t" + QString::number(action->layer) + "\t" + action->persona + "\t";
        break;
    case PrintText:
        body = "\t" + action->persona + "\t" + action->text + "\t";
        break;
    case SetSELECT:
        body = "\t" + action->answer1 + "\t" + action->answer2 + "\t";
        break;
    case EndBGM:
    case EndRoll:
        body = "\t" + action->file_name + "\t";
        break;
    case MoveSom:
        body = "\t" + QString::number(action->layer) + "\t";
        break;
    }

    return start + body + back_time(action->end) + ";\n\n";
}
