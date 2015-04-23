#include "qscript.h"
#include <QTextStream>
#include <QFile>
#include <QMetaEnum>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include "QDateTime"

#define delete_if_not_NULL(obj) \
    if (obj != NULL) { delete obj; obj = NULL; }

enum {
    SECS_PER_DAY = 86400,
    MSECS_PER_DAY = 86400000,
    SECS_PER_HOUR = 3600,
    MSECS_PER_HOUR = 3600000,
    SECS_PER_MIN = 60,
    MSECS_PER_MIN = 60000,
    TIME_T_MAX = 2145916799,  // int maximum 2037-12-31T23:59:59 UTC
    JULIAN_DAY_FOR_EPOCH = 2440588 // result of julianDayFromDate(1970, 1, 1)
};

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

bool compareActionByStart(QScript::QScriptAction* a1, QScript::QScriptAction* a2)
{
    return a1->start < a2->start;
}

bool compareActionByEnd(QScript::QScriptAction* a1, QScript::QScriptAction* a2)
{
    return a1->end < a2->end;
}

QScript::QScript(QFileSystem* fs, QObject *parent) :
    QObject(parent)
{
    this->fs = fs;
}

QScript::~QScript()
{
    // clear screent
    for (int i=0 ; i<10 ; i++)
        emit SetLayerImage(i, NULL);

    foreach(QScriptAction* action, actions)
    {
        if (action->running)
        {
            switch (action->action)
            {
            case PlayVoice:
                delete_if_not_NULL(action->sync_timer);
                delete_if_not_NULL(action->sync[0]);
                delete_if_not_NULL(action->sync[1]);
                delete_if_not_NULL(action->sync[2]);
            case PlayBgm:
            case PlayES:
            case PlaySe:
            case EndBGM:
                break;
            case CreateBG:
                delete_if_not_NULL(action->image);
                break;
            case PrintText:
            case SetSELECT:
            case BlackFade:
            case WhiteFade:
            case PlayMovie:
            case EndRoll:
            case Next:
                break;
            default:
                break;
            }
        }
        delete action;
    }
    delete current_timer;
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
    qSort(actions.begin(), actions.end(), compareActionByStart);
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
        default:
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
        default:
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
    this->start_time = QDateTime::currentMSecsSinceEpoch();
    if (actions.length() > 0)
        action_sheduler(actions.at(0));
}

void QScript::action_sheduler(void* current_action)
{
    // TODO: preload data

    checkStopActions();

    int startIdx = actions.indexOf((QScriptAction*)current_action);
    int endIdx = actions.length();
    qint64 time = QDateTime::currentMSecsSinceEpoch() - this->start_time;

    for (int i=startIdx ; i<endIdx ; i++)
    {
        QScriptAction* action = actions.at(i);

        if (action->running)
            continue;

        if (action->start > time)
        {
            current_timer = new QSingleActionTimer(action, action->start - time, this, SLOT(action_sheduler(void*)));
            break;
        }

        runAction(action);
    }
    if (queue_stop.length() > 0)
    {
        qSort(queue_stop.begin(), queue_stop.end(), compareActionByEnd);
        //QScriptAction* action = queue_stop.at(0);
        //new QSingleActionTimer(action, action->end - time, this, SLOT(action_sheduler(void*)));
    }
}

void QScript::checkStopActions()
{
    qint64 time = QDateTime::currentMSecsSinceEpoch() - this->start_time;
    int l = queue_stop.length();
    for (int i=0 ; i<l ; i++)
    {
        QScriptAction* action = queue_stop.at(i);

        if (action->end > time)
            break;

        if ((action->running) && (action->end <= time))
        {
            action_stop(action);
            action->runned = true;
            action->running = false;
            queue_stop.removeOne(action);
            i--;
            l--;
        }
    }
}

void QScript::runAction(QScriptAction* action)
{
#ifdef QT_DEBUG
    QMetaEnum en = staticMetaObject.enumerator(0);
    qDebug() << "Start action" << en.valueToKey(action->action) << " at " << action->start <<
                " rel_end:" << action->end - action->start;
#endif

    QString fn;
    if (action->file_name.length() > 0)
        fn = fs->getPackDir() + fs->normalize_name(action->file_name);

    switch (action->action)
    {
    case PlayVoice:
        if (this->latest_bg_fn.length() > 0)
        {
            action->sync_phaze = 0;
            QString fns = fs->getPackDir() + this->latest_bg_fn + action->persona.toUpper() + '.';
            action->sync[0] = new QImage(fns + 'A' + ".PNG");
            action->sync[1] = new QImage(fns + 'B' + ".PNG");
            action->sync[2] = new QImage(fns + 'C' + ".PNG");
            action->sync_timer = new QActionTimer(action, 100, this, SLOT(action_sync(void*)));
        }
#ifdef QT_DEBUG
        qDebug() << "Voice:" << action->persona;
#endif
    case PlayBgm:
    case PlayES:
    case PlaySe:
    case EndBGM:
        //action->sound = new QSound(fn);
        //action->sound->play();
        break;
    case CreateBG:
        this->latest_bg_fn = action->file_name;
        action->image = new QImage(fn);
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
    action->running = true;
    queue_stop.append(action);
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
    case PlayVoice:
        if (this->latest_bg_fn.length() > 0)
        {
            emit SetLayerImage(1, NULL);
            delete_if_not_NULL(action->sync_timer);
            delete_if_not_NULL(action->sync[0]);
            delete_if_not_NULL(action->sync[1]);
            delete_if_not_NULL(action->sync[2]);
        }
        break;
    case PlayBgm:
    case PlayES:
    case PlaySe:
    case EndBGM:
        //delete_if_not_NULL(action->sound);
        break;
    case CreateBG:
        this->latest_bg_fn.clear();
        emit SetLayerImage(0, NULL);
        delete_if_not_NULL(action->image);
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

void QScript::action_sync(void* data)
{
    QScriptAction* action = (QScriptAction*)data;
    switch (action->action)
    {
    case PlayVoice:
        emit SetLayerImage(1, action->sync[action->sync_phaze]);
        action->sync_phaze++;
        if (action->sync_phaze >= 3)
            action->sync_phaze = 0;
    default:
        break;
    }
}

void QScript::add_action_by_ors(QString action, QStringList params)
{
    QMetaEnum en = staticMetaObject.enumerator(0);
    /*actions.append(new QScriptAction((Action)en.keyToValue(action.toStdString().c_str()),
                                     &params, fs, this));*/
    QScriptAction* action_rec = new QScriptAction;

    action_rec->runned = false;
    action_rec->running = false;
    action_rec->sync_timer = NULL;
    action_rec->sync[0] = NULL;
    action_rec->sync[2] = NULL;
    action_rec->sync[3] = NULL;
    action_rec->image = NULL;

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
