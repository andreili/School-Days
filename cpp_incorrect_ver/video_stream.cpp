#include "video_stream.h"

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

VideoStream::VideoStream()
{
    this->m_av_ctx = NULL;
    this->m_io_buf = NULL;
    this->m_io_ctx = NULL;
    this->m_codec_ctx = NULL;

    av_register_all();
}

VideoStream::~VideoStream()
{
}

int readFunction(void* opaque, uint8_t* buf, int buf_size)
{
    Stream* stream = (Stream*)opaque;
    int numBytes = stream->read(buf, buf_size);
    return numBytes;
}

int64_t seekFunction(void* opaque, int64_t offset, int whence)
{
    if (whence == AVSEEK_SIZE)
        return -1; // I don't know "size of my handle in bytes"
    Stream* stream = (Stream*)opaque;
    return stream->seek(offset, spBegin);
}

bool VideoStream::load(Stream *stream)
{
    this->m_av_ctx = avformat_alloc_context();

    this->m_io_buf = (uint8_t*)av_malloc(INBUF_SIZE);
    this->m_io_ctx = avio_alloc_context(this->m_io_buf, INBUF_SIZE, 0, stream, readFunction, NULL, seekFunction);
    this->m_av_ctx->pb = this->m_io_ctx;

    if (avformat_open_input(&this->m_av_ctx, "dummyFilename", NULL, NULL) != 0)
    {
        ERROR_MESSAGE("Video: unable to open file\n");
        return false;
    }

    this->m_video_stream_idx = this->m_av_ctx->nb_streams;
    for (uint32_t i=0 ; i<this->m_av_ctx->nb_streams ; i++)
        if (this->m_av_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            this->m_video_stream_idx = i;
            break;
        }
    if (this->m_video_stream_idx == (int)this->m_av_ctx->nb_streams)
    {
        ERROR_MESSAGE("Video: Didn't find a video stream\n");
        return false;
    }
    this->m_codec_ctx = this->m_av_ctx->streams[this->m_video_stream_idx]->codec;
    printf("Frame rate %i/%i\n", this->m_av_ctx->streams[this->m_video_stream_idx]->r_frame_rate.num, this->m_av_ctx->streams[this->m_video_stream_idx]->r_frame_rate.den);

    AVCodec *codec;
    codec = avcodec_find_decoder(this->m_codec_ctx->codec_id);
    if (codec == NULL)
    {
        ERROR_MESSAGE("Video: Codec not found\n");
        return false;
    }

    if (avcodec_open2(this->m_codec_ctx, codec, NULL) < 0)
    {
        ERROR_MESSAGE("Video: Could not open codec\n");
        return false;
    }

    // Allocate video frame
    this->m_frame = avcodec_alloc_frame();
    this->m_frameRGB = avcodec_alloc_frame();

    int numBytes;
    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(PIX_FMT_RGB24, this->m_codec_ctx->width,
                                  this->m_codec_ctx->height);
    this->m_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

    avpicture_fill((AVPicture *)this->m_frameRGB, this->m_buffer, PIX_FMT_RGB24,
                   this->m_codec_ctx->width, this->m_codec_ctx->height);

    this->m_frameIdx = 0;

    return true;
}

void VideoStream::play()
{
}

void VideoStream::stop()
{
}

void VideoStream::close()
{
    avcodec_free_frame(&this->m_frame);
    avcodec_free_frame(&this->m_frameRGB);
    avcodec_close(this->m_codec_ctx);
    av_free(this->m_codec_ctx);
    av_free(this->m_io_buf);
    av_free(this->m_buffer);
    avio_close(this->m_io_ctx);
    avformat_free_context(this->m_av_ctx);
}



void VideoStream::decode(const char *outfilename)
{
    AVPacket packet;
    int frameFinished;

    while(av_read_frame(this->m_av_ctx, &packet)>=0)
    {
        if(packet.stream_index==this->m_video_stream_idx)
        {
            avcodec_decode_video2(this->m_codec_ctx, this->m_frame, &frameFinished, &packet);

            if(frameFinished)
            {
                // Convert the image from its native format to RGB
                //img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, (AVPicture*)pFrame, this->m_codec_ctx->pix_fmt, this->m_codec_ctx->width, this->m_codec_ctx->height);

                // Save the frame to disk
                this->m_frameIdx++;
            }
        }
        av_free_packet(&packet);
    }
}
