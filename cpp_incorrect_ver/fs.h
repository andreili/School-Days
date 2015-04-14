#ifndef _FS_H_
#define _FS_H_

#include <stdlib.h>
#include "gpk.h"
#include <vector>

#ifdef _WIN32
#define PACKS_DIR ".\\packs\\"
#elif defined __unix__
#define PACKS_DIR "./packs/"
#else
    #error here!
#endif

using namespace std;

typedef vector<GPK*> GPK_list_t;

class FS
{
private:
    char        m_root[MAX_PATH];
    char        m_ext[MAX_PATH];
    GPK_list_t  m_packs;
public:
    FS(const char *root_dir);
    ~FS();

    void set_ext(const char *ext);

    bool add_packs(const char *path);
    bool mount_pack(const char *file_name);

    Stream *open(const char *file_name);
};

#endif
