#include "defines.h"
#include "settings.h"
#include "stream.h"
#include "fs.h"
#include <stdlib.h>

Settings::Settings()
{
}

Settings::~Settings()
{
    this->m_settings.clear();
    uint32_t size = this->m_inis.size();
    for (uint32_t i = 0 ; i<size ; i++)
        delete this->m_inis.at(i);
    this->m_inis.clear();
}

bool Settings::load(const char *file_name)
{
    Stream *str = global.fs->open(file_name);
    if (str->getFileStreamHandle() == INVALID_HANDLE_VALUE)
    {
        ERROR_MESSAGE("Settings: File %s doesn\'t opened\n", file_name);
        return false;
    }

    Parser *ini = new Parser(str);

    uint32_t count = ini->get_count();
    for (uint32_t i=0 ; i<count ; i++)
    {
        this->m_settings[ini->get_name(i)].value.as_char = ini->get_value(i);
        this->m_settings[ini->get_name(i)].type = VALUE_STRING;
    }

    this->m_inis.push_back(ini);
    delete str;

    return true;
}

setting_t *Settings::get_value(const char *name)
{
    settings_t::iterator end = this->m_settings.end();
    for (settings_t::iterator it=this->m_settings.begin() ; it!=end ; it++)
    {
        //printf("\t%s=%s\n", it->first, it->second.value.as_char);
        if (strcmp(it->first, name) == 0)
        {
            return &it->second;
        }
    }
    return NULL;
}

const char *Settings::get_string(const char *name)
{
    if (setting_t *val = this->get_value(name))
    {
        return val->value.as_char;
    }
    else
        return NULL;
}

bool Settings::get_bool(const char *name)
{
    if (setting_t *val = this->get_value(name))
    {
        if (val->type != VALUE_INT)
        {
            val->value.as_int = atoi(val->value.as_char);
            val->type = VALUE_INT;
        }
        return val->value.as_int;
    }
    else
        return false;
}

int Settings::get_int(const char *name)
{
    if (setting_t *val = this->get_value(name))
    {
        if (val->type != VALUE_INT)
        {
            val->value.as_int = atoi(val->value.as_char);
            val->type = VALUE_INT;
        }
        return val->value.as_int;
    }
    else
        return -1;
}

float Settings::get_float(const char *name)
{
    if (setting_t *val = this->get_value(name))
    {
        if (val->type != VALUE_FLOAT)
        {
            val->value.as_float = atof(val->value.as_char);
            val->type = VALUE_FLOAT;
        }
        return val->value.as_int;
    }
    else
        return -1.0;
}


void Settings::set_int(const char *name, int value)
{
    if (setting_t *val = this->get_value(name))
    {
#ifdef SD_DEBUG
        NORMA_MESSAGE("Settings: set %s = %i\n", name, value);
#endif
        val->value.as_int = value;
        val->type = VALUE_INT;
    }
}
