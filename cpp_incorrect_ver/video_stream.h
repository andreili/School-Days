#ifndef _VIDEO_STREAM_H
#define _VIDEO_STREAM_H

#include "defines.h"
#include "stream.h"
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
}

class VideoStream
{
private:
    AVFormatContext     *m_av_ctx;
    uint8_t             *m_io_buf;
    AVIOContext         *m_io_ctx;
    AVCodecContext      *m_codec_ctx;
    int                 m_video_stream_idx;
    AVFrame             *m_frame;
    AVFrame             *m_frameRGB;
    uint8_t             *m_buffer;
    int                 m_frameIdx;
public:
    VideoStream();
    ~VideoStream();

    bool load(Stream *stream);
    void decode(const char *outfilename);
    void play();
    void stop();
    void close();
};

#endif
