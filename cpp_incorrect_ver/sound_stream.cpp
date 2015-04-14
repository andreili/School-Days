#include "defines.h"
#include "sound_stream.h"
#include <string>

#define NUM_OF_DYNBUF	200 		// num buffers in queue
#define DYNBUF_SIZE		1024*100	// Buffer size

SoundStream::SoundStream()
{
}

SoundStream::~SoundStream()
{
    TBuf::iterator end = this->m_buffers.end();
    for (TBuf::iterator i = this->m_buffers.begin(); i != end; i++)
        alDeleteBuffers(1, &i->second.ID);
}

ALboolean SoundStream::CheckALError()
{
    ALenum ErrCode;
    string Err = "OpenAL error: ";
    if ((ErrCode = alGetError()) != AL_NO_ERROR)
    {
        Err += alGetString(ErrCode);
        Err +="\n";
        ERROR_MESSAGEex(Err.c_str());
        return AL_FALSE;
    }
    return AL_TRUE;
}

bool SoundStream::load(Stream *stream, bool looped)
{
    // Create source
    alGenSources(1, &this->m_sourceID);
    if (!CheckALError())
    {
        ERROR_MESSAGE("SoundStream: Unable to create source\n");
        return false;
    }

    ALfloat mVel[] = {0., 0., 0.};
    ALfloat mPos[] = {0., 0., 0.};

    alSourcef (this->m_sourceID, AL_PITCH,		1.0f);
    alSourcef (this->m_sourceID, AL_GAIN,		1.0f);
    alSourcefv(this->m_sourceID, AL_POSITION,	mPos);
    alSourcefv(this->m_sourceID, AL_VELOCITY,	mVel);

    alSourcef(this->m_sourceID, AL_GAIN, 1.0f);
    this->m_loop = looped;

    return load_ogg(stream);
}

size_t ReadOgg(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    Stream *File = (Stream*)datasource;
    return File->read(ptr, size * nmemb);
}

int SeekOgg(void *datasource, ogg_int64_t offset, int whence)
{
    Stream *File = (Stream*)datasource;
    ESeekMethod Dir;
    switch (whence)
    {
    case SEEK_SET:
        Dir = spBegin;
        break;
    case SEEK_CUR:
        Dir = spCurrent;
        break;
    case SEEK_END:
        Dir = spEnd;
        break;
    default:
        return -1;
    }
    return (File->seek(offset, Dir)) ? 0 : -1;
}

long TellOgg(void *datasource)
{
    Stream *File = (Stream*)datasource;
    return File->getPosition();
}

int CloseOgg(void *datasource)
{
    return 0;
}

bool SoundStream::load_ogg(Stream *stream)
{
    int				i, DynBuffs = 1, BlockSize;
    // OAL specific
    SndInfo			buffer;
    ALuint			BufID = 0;
    // OggVorbis specific structures
    ov_callbacks	cb;

    // Fill cb struct
    cb.close_func	= CloseOgg;
    cb.read_func	= ReadOgg;
    cb.seek_func	= SeekOgg;
    cb.tell_func	= TellOgg;

    // Create OggVorbis_File struct
    this->m_VF = new OggVorbis_File;

    // Open Ogg file

    // Generate local buffers
    if (ov_open_callbacks(stream, this->m_VF, NULL, -1, cb) < 0)
    {
        // This is not ogg bitstream. Return
        ERROR_MESSAGE("SoundStream: This is not ogg bitstream\n");
        return false;
    }

    // Check for existance of sound
    BlockSize	= DYNBUF_SIZE;
    DynBuffs	= NUM_OF_DYNBUF;
    alSourcei(this->m_sourceID, AL_LOOPING, AL_FALSE);

    // Return vorbis_comment and vorbis_info structures
    this->m_comment		= ov_comment(this->m_VF, -1);
    this->m_info		= ov_info(this->m_VF, -1);

    // Fill buffer infos
    buffer.Rate	  = this->m_info->rate;
    buffer.Format = (this->m_info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    this->m_proceed = 0;
    alGetSourcei(this->m_sourceID, AL_BUFFERS_PROCESSED, &this->m_proceed);
    // Fill buffers with data each block by DYNBUF_SIZE bytes
    if (!BufID)
    {
        for (i = 0; i < DynBuffs; i++)
        {
            // Create buffers
            alGenBuffers(1, &buffer.ID);
            if (!CheckALError())
            {
                ERROR_MESSAGE("SoundStream: unable to create buffer\n");
                return false;
            }
            this->m_buffers[buffer.ID] = buffer;
            // Read amount (DYNBUF_SIZE) data into each buffer
            if (!ReadOggBlock(buffer.ID, BlockSize))
            {
                //NORMA_MESSAGE("SoundStream: EOF\n");
                break;
            }
            if (!CheckALError())
            {
                ERROR_MESSAGE("SoundStream: unable to read buffer\n");
                return false;
            }

            //alSourcei(this->m_sourceID, AL_BUFFER, buffer.ID);
        }
    }
    else
    {
        alSourcei(this->m_sourceID, AL_BUFFER, this->m_buffers[BufID].ID);
    }

    alSourcei (this->m_sourceID, AL_LOOPING,	this->m_loop);
    //alSourceQueueBuffers(this->m_sourceID, this->m_buffers.size()), this->m_buffers);

    return true;
}

bool SoundStream::ReadOggBlock(ALuint BufID, size_t Size)
{
    // vars
    int			current_section;
    size_t		TotalRet = 0, ret = 1;
    char		*PCM;

    if (Size < 1) return false;
    PCM = new char[Size];

    // Read loop
    while ((TotalRet < Size) && (ret != 0))
    {
        ret = ov_read(this->m_VF, PCM + TotalRet, Size - TotalRet, 0, 2, 1, &current_section);

        switch (ret)
        {
        case OV_HOLE:
            ERROR_MESSAGE("SoundStream: OGG interruption in the data\n");
            return false;
            break;
        case OV_EBADLINK:
            ERROR_MESSAGE("SoundStream: OGG invalid stream section\n");
            return false;
            break;
        case OV_EINVAL:
            ERROR_MESSAGE("SoundStream: OGG initial file headers couldn't be read or are corrupt\n");
            return false;
            break;
        case 0:
            // if end of file or read limit exceeded
            break;
        default:
            TotalRet += ret;
            break;
        }
    }
    if (TotalRet > 0)
    {
        alBufferData(BufID, this->m_buffers[BufID].Format, (void *)PCM, TotalRet, this->m_buffers[BufID].Rate);
        //printf("Buffer: format %u, rate %u\n", this->m_buffers[BufID].Format, this->m_buffers[BufID].Rate);
        if (!CheckALError())
        {
            ERROR_MESSAGE("SoundStream: unable to send buffer\n");
            return false;
        }
        alSourceQueueBuffers(this->m_sourceID, 1, &BufID);
    }
    delete [] PCM;
    return (ret > 0);
}

void SoundStream::play()
{
    alSourcePlay(this->m_sourceID);
}

void SoundStream::stop()
{
    alSourceStop(this->m_sourceID);
}

void SoundStream::close()
{
    alSourceStop(this->m_sourceID);
    if (alIsSource(this->m_sourceID))
        alDeleteSources(1, &this->m_sourceID);
    if (!this->m_VF)
    {
        ov_clear(this->m_VF);
        delete this->m_VF;
    }
}
