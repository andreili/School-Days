#ifndef _PARSER_H_
#define _PARSER_H_

#include "stream.h"
#include <vector>

using namespace std;

typedef struct
{
    char *name;
    char *value;
} value_t;

typedef vector<value_t*> values_t;

class Parser
{
private:
    values_t    m_values;

    void _parse(const char *buf, uint32_t size);
public:
    Parser(const char *buf, uint32_t size);
    Parser(Stream *stream);
    ~Parser();

    uint32_t get_count();
    const char *get_name(uint32_t idx);
    const char *get_value(uint32_t idx);
    const char *get_value(const char *name);
};

#endif
