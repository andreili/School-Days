#include <QCoreApplication>
#include "../classes/qfilesystem.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*if (argc != 2)
        return 1;*/

    //QFileSystem* fs = new QFileSystem(argv[1]);
    QFileSystem* fs = new QFileSystem("h:\\Games\\Школьные Дни HQ\\");
    fs->unpack_all();

    return a.exec();
}
