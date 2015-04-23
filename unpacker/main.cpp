#include <QCoreApplication>
#include "../classes/qfilesystem.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*if (argc != 2)
        return 1;*/

    //QFileSystem* fs = new QFileSystem(argv[1]);
    QFileSystem* fs = new QFileSystem("/media/work/Dev/Games/SD");
    fs->unpack_all();

    return a.exec();
}
