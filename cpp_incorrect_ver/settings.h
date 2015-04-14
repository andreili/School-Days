#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "parser.h"
#include <map>
#include <vector>

using namespace std;

typedef enum
{
    VALUE_STRING,
    VALUE_INT,
    VALUE_FLOAT
} EValueType;

typedef struct
{
    union
    {
        int32_t as_int;
        const char *as_char;
        float as_float;
    } value;
    EValueType type;
} setting_t;

typedef map<const char*, setting_t> settings_t;
typedef vector<Parser*> settings_inis_t;

class Settings
{
private:
    settings_t  m_settings;
    settings_inis_t m_inis;

    setting_t *get_value(const char *name);
public:
    Settings();
    ~Settings();

    bool load(const char *file_name);

    const char *get_string(const char *name);
    bool get_bool(const char *name);
    int get_int(const char *name);
    float get_float(const char *name);

    void set_int(const char *name, int value);
};

#endif
