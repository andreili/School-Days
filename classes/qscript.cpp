#include "qscript.h"
#include <QTextStream>
#include <QFile>
#include <QMetaEnum>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

QScript::QScript(QObject *parent) :
    QObject(parent)
{
}

QScript::~QScript()
{
    foreach(QScriptAction* action, actions)
    {
        delete action;
    }
    foreach(QTimer* timer, timers)
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

        QMetaEnum en = QScriptAction::staticMetaObject.enumerator(0);
        action_obj["action"] = QString(en.valueToKey(action->getAction()));
        action_obj["start"] = action->getStartTime();

        switch (action->getAction())
        {
        case QScriptAction::SkipFRAME:
        case QScriptAction::Next:
            obj[idx] = action_obj;
            obj.append(action_obj);
            idx++;
            continue;
        case QScriptAction::PrintText:
            action_obj["text"] = action->getText();
            action_obj["persona"] = action->getPersona();
            break;
        case QScriptAction::SetSELECT:
            action_obj["answer1"] = action->getAnswer1();
            action_obj["answer2"] = action->getAnswer2();
            break;
        case QScriptAction::PlaySe:
            action_obj["layer"] = action->getLayer();
        case QScriptAction::CreateBG:
        case QScriptAction::PlayMovie:
        case QScriptAction::PlayBgm:
        case QScriptAction::PlayES:
        case QScriptAction::EndBGM:
        case QScriptAction::EndRoll:
            action_obj["file"] = action->getFile();
            break;
        case QScriptAction::BlackFade:
        case QScriptAction::WhiteFade:
            action_obj["dir"] = action->getDirection();
            break;
        case QScriptAction::PlayVoice:
            action_obj["file"] = action->getFile();
            action_obj["layer"] = action->getLayer();
            action_obj["persona"] = action->getPersona();
            break;
        case QScriptAction::MoveSom:
            break;
        }

        action_obj["end"] = action->getEndTime();
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
        switch (action->getAction())
        {
        case QScriptAction::PrintText:
            text_stream << action->getPersona() << endl;
            text_stream << action->getText() << endl;
            break;
        case QScriptAction::SetSELECT:
            text_stream << action->getAnswer1() << endl;
            text_stream << action->getAnswer2() << endl;
            break;
        case QScriptAction::SkipFRAME:
        case QScriptAction::CreateBG:
        case QScriptAction::BlackFade:
        case QScriptAction::WhiteFade:
        case QScriptAction::PlayMovie:
        case QScriptAction::PlayBgm:
        case QScriptAction::PlaySe:
        case QScriptAction::PlayES:
        case QScriptAction::PlayVoice:
        case QScriptAction::Next:
        case QScriptAction::EndBGM:
        case QScriptAction::EndRoll:
        case QScriptAction::MoveSom:
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
        switch (action->getAction())
        {
        case QScriptAction::PrintText:
            action->setPersona(text_stream.readLine());
            action->setText(text_stream.readLine());
            break;
        case QScriptAction::SetSELECT:
            action->setAnswer1(text_stream.readLine());
            action->setAnswer2(text_stream.readLine());
            break;
        case QScriptAction::SkipFRAME:
        case QScriptAction::CreateBG:
        case QScriptAction::BlackFade:
        case QScriptAction::WhiteFade:
        case QScriptAction::PlayMovie:
        case QScriptAction::PlayBgm:
        case QScriptAction::PlaySe:
        case QScriptAction::PlayES:
        case QScriptAction::PlayVoice:
        case QScriptAction::Next:
        case QScriptAction::EndBGM:
        case QScriptAction::EndRoll:
        case QScriptAction::MoveSom:
            break;
        }

        ors_stream << action->toString();
    }

    text.close();
    ors_stream.flush();
    ors.close();
}

void QScript::add_action_by_ors(QString action, QStringList params)
{
    QMetaEnum en = QScriptAction::staticMetaObject.enumerator(0);
    actions.append(new QScriptAction((QScriptAction::Action)en.keyToValue(action.toStdString().c_str()), &params, this));
}
