#include "qscriptaction.h"

int convert_time(QString time)
{
    return 0;
}

QScriptAction::QScriptAction(Action action, QStringList *params, QObject *parent) :
    QObject(parent)
{
    this->layer = -1;

    this->action = action;
    switch (action)
    {
    case SkipFRAME:
        this->start = convert_time(params->at(1));
        break;
    case CreateBG:
        this->start = convert_time(params->at(1));
        this->file_name = params->at(3);
        this->end = convert_time(params->at(4));
        break;
    case BlackFade:
    case WhiteFade:
        this->start = convert_time(params->at(1));
        this->direction = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case PlayMovie:
        this->start = convert_time(params->at(1));
        this->file_name = params->at(2);
        this->end = convert_time(params->at(4));
        break;
    case PlaySe:
        this->start = convert_time(params->at(1));
        this->layer = params->at(2).toInt();
        this->file_name = params->at(3);
        this->end = convert_time(params->at(4));
        break;
    case PlayBgm:
        this->start = convert_time(params->at(1));
        this->file_name = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case PlayVoice:
        this->start = convert_time(params->at(1));
        this->file_name = params->at(2);
        if (params->count() == 4)
            this->end = convert_time(params->at(3));
        else if (params->count() == 5)
        {
            this->layer = params->at(3).toInt();
            this->end = convert_time(params->at(4));
        }
        else
        {
            this->layer = params->at(3).toInt();
            this->persona = params->at(4);
            this->end = convert_time(params->at(5));
        }
        break;
    case PrintText:
        this->start = convert_time(params->at(1));
        this->persona = params->at(2);
        this->text = params->at(3);
        this->end = convert_time(params->at(4));
        break;
    case SetSELECT:
        this->start = convert_time(params->at(1));
        this->answer1 = params->at(2);
        this->answer2 = params->at(3);
        this->end = convert_time(params->at(4));
        break;
    case Next:
        this->start = convert_time(params->at(1));
        break;
    case EndBGM:
        this->start = convert_time(params->at(1));
        this->file_name = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case EndRoll:
        this->start = convert_time(params->at(1));
        this->file_name = params->at(2);
        this->end = convert_time(params->at(3));
        break;
    case MoveSom:
        this->start = convert_time(params->at(1));
        this->layer = params->at(2).toInt();
        this->end = convert_time(params->at(3));
        break;
    }
}
