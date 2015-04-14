#include "qscript.h"
#include <QTextStream>
#include <QFile>
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
                line.split(QRegExp("[=\\[\\]\\t;]"), QString::SkipEmptyParts));
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
            break;
        }
    }

    text_stream.flush();
    text.close();
}

void QScript::add_action_by_ors(QString action, QStringList params)
{
    if (action.compare("CreateBG") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::CreateBG, &params, this));
    }
    else if (action.compare("PlayMovie") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::PlayMovie, &params, this));
    }
    else if (action.compare("PlaySe") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::PlaySe, &params, this));
    }
    else if (action.compare("PrintText") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::PrintText, &params, this));
    }
    else if (action.compare("BlackFade") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::BlackFade, &params, this));
    }
    else if (action.compare("WhiteFade") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::WhiteFade, &params, this));
    }
    else if (action.compare("PlayVoice") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::PlayVoice, &params, this));
    }
    else if (action.compare("PlayBgm") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::PlayBgm, &params, this));
    }
    else if (action.compare("SkipFRAME") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::SkipFRAME, &params, this));
    }
    else if (action.compare("SetSELECT") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::SetSELECT, &params, this));
    }
    else if (action.compare("Next") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::Next, &params, this));
    }
    else if (action.compare("EndBGM") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::EndBGM, &params, this));
    }
    else if (action.compare("EndRoll") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::EndRoll, &params, this));
    }
    else if (action.compare("MoveSom") == 0)
    {
        actions.append(new QScriptAction(QScriptAction::MoveSom, &params, this));
    }
    else qDebug() << "Unknown action: " << action;
}
