#include "qscript.h"
#include <QTextStream>
#include <QFile>
#include <QMetaEnum>
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

void QScript::load_from_ORS(QIODevice *io)
{
    QTextStream stream(io);
    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        if ((line.length() == 0) || (line[0] != '['))
            continue;

        this->add_action_by_ors(line.split('\t')[0].split(QRegExp("[:=\\[\\];]"), QString::SkipEmptyParts)[0],
                line.split('\t', QString::SkipEmptyParts));
    }
}

QString QScript::serialize()
{
    return "";
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

    QFile ors(file_name + ".ORS");
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
