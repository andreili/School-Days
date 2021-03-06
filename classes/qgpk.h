#ifndef QGPK_H
#define QGPK_H

#include <QObject>
#include <QStringList>
#include <QList>

#define GPK_TAILER_IDENT0	"STKFile0PIDX"
#define GPK_TAILER_IDENT1	"STKFile0PACKFILE"

const unsigned char CIPHERCODE[16] = {0x82, 0xEE, 0x1D, 0xB3,
                                      0x57, 0xE9, 0x2C, 0xC2,
                                      0x2F, 0x54, 0x7B, 0x10,
                                      0x4C, 0x9A, 0x75, 0x49
                                     };

#pragma pack (1)
typedef struct
{
    char        sig0[12];
    quint32     pidx_length;
    char        sig1[16];
} GPKsig;

typedef struct
{
    quint16     sub_version;		// same as script.gpk.* suffix
    quint16     version;			// major version(always 1)
    quint16     zero;               // always 0
    quint32     offset;             // pidx data file offset
    quint32     comprlen;			// compressed pidx data length
    char        dflt[4];			// magic "DFLT" or "    "
    quint32     uncomprlen;         // raw pidx data length(if magic isn't DFLT, then this filed always zero)
    char        comprheadlen;		// pidx data header length
} GPKEntryHeader;

typedef struct
{
    QString         name;
    GPKEntryHeader  header;
} GPKentry;
#pragma pack ()

class QGPKFile;

class QGPK : public QObject
{
    Q_OBJECT
public:
    explicit QGPK(QObject *parent = 0);

    bool load(QString file_name);
    QGPKFile* open(QString filename);
    QStringList list(QString mask);

    QString getName();

    void unpack_all(QString dir);

signals:

public slots:

private:
    QList<GPKentry> entries;
    QString name;
};

#endif // QGPK_H
