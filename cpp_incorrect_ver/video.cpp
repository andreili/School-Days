#include "video.h"
#include "stream.h"

Video::Video()
{
}

Video::~Video()
{
#ifdef SD_DEBUG
    printf("Video: free\n");
#endif
}

bool Video::init()
{
    //av_log_set_callback( ffmpeg_avcodec_log );

    //decode("test%d.pgm", "extr/00-00/00-00-A00/00-00-A00-000.WMV");
    return true;
}
