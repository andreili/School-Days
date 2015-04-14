#include "parser.h"
#include <string>

#ifdef SD_DEBUG
#include <stdio.h>
#endif

Parser::Parser(const char *buf, uint32_t size)
{
    _parse(buf, size);
}

Parser::Parser(Stream *stream)
{
    uint32_t sz = stream->getSize();
    char *buf = new char[sz];
    stream->seek(0, spBegin);
    stream->read(buf, sz);
    _parse(buf, sz);
    delete buf;
}

Parser::~Parser()
{
#ifdef SD_DEBUG
    printf("Parser: free\n");
#endif
    uint32_t size = this->m_values.size();
    for (uint32_t i=0 ; i<size ; i++)
    {
        value_t *val = this->m_values.at(i);
        delete val->name;
        delete val->value;
        delete val;
    }
    this->m_values.clear();
}

void Parser::_parse(const char *buf, uint32_t size)
{
    const char *end = buf + size,
                *c_data = buf;
    while (c_data < end)
    {
        if (c_data[0] == '[')
        {
            c_data++;
            int pos = strchr(c_data, ']') - c_data;
            value_t *val = new value_t;
            val->name = new char[pos + 1];
            strncpy(val->name, c_data, pos);
            val->name[pos] = 0;
            c_data += pos + 2;

            if (c_data[0] == '"')
            {
                // string data
                c_data++;
                pos = strchr(c_data, '"') - c_data;
            }
            else
            {
                pos = strchr(c_data, '\r') - c_data;
                if (pos < 0)
                    pos = strchr(c_data, '\n') - c_data;
            }
            val->value = new char[pos + 1];
            val->value[pos] = 0;
            strncpy(val->value, c_data, pos);

            c_data += pos;
            if (c_data[0] == '"')
                c_data++;

            this->m_values.push_back(val);
        }
        c_data++;
    }
}
uint32_t Parser::get_count()
{
    return this->m_values.size();
}

const char *Parser::get_name(uint32_t idx)
{
    return this->m_values.at(idx)->name;
}

const char *Parser::get_value(uint32_t idx)
{
    return this->m_values.at(idx)->value;
}

const char *Parser::get_value(const char *name)
{
    uint32_t size = this->m_values.size();
    for (uint32_t i=0 ; i<size ; i++)
        if (strcmp(this->m_values.at(i)->name, name) == 0)
            return this->m_values.at(i)->value;
    return NULL;
}
