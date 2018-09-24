//
// Created by Joseph Chen
// jocodoma@gmail.com
//

#include "audioPlayer.h"
#include "logger.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define LOG_TAG "[audioPlayer.c]"

#define SLASSERT(x)                   \
  do {                                \
    assert(SL_RESULT_SUCCESS == (x)); \
    (void)(x);                        \
  } while (0)

#define BUFFER_QUEUE_LEN    4

typedef struct SampleFormat {
    uint32_t sampleRate;
    uint32_t framesPerBuf;
    uint16_t numChannels;
    uint16_t pcmFormat;  // 8 bit, 16 bit, 24 bit ...
    uint32_t representation;  // android extensions
} SampleFormat;

// SL buffer queue player interfaces
typedef struct SLBufferQueuePlayer {
    SLObjectItf outputMixObjectItf;
    SLObjectItf playerObjectItf;
    SLPlayItf playItf;
    SLAndroidSimpleBufferQueueItf playBufferQueueItf;
    SampleFormat sampleInfo;
    uint32_t bufferSize;
    short *buffer;
} SLPlayer;

typedef struct SLEngine {
    SLmilliHertz fastPathSampleRate;
    uint32_t fastPathFramesPerBuf;
    uint16_t sampleChannels;
    uint16_t bitsPerSample;
    SLObjectItf slEngineObj;
    SLEngineItf slEngineItf;
} SLEngine;

SLEngine engine;
SLPlayer player;
SampleFormat sampleFormat;

FILE  *file = NULL;
bool isEOF = false;

static void bufferQueuePlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == player.playBufferQueueItf);
    assert(NULL == context);

    int bytes = fread(player.buffer, 1, player.bufferSize*sizeof(short), file);
    // int bytes = processGnFiveBandEq(player.buffer, engine.fastPathFramesPerBuf);
    SLresult result = (*bq)->Enqueue(bq, player.buffer, bytes);

    // LOG_DEBUG(" === === === Enqueue === === === %d", bytes);

    // ToDo: need to handle EOF
    // Currently we listen to "SL_RESULT_PARAMETER_INVALID" to handle EOF
    if(SL_RESULT_PARAMETER_INVALID == result) {
        isEOF = true;
    } else {
        SLASSERT(result);
    }
}

static void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX* pFormat, SampleFormat* pSampleInfo_)
{
    assert(pFormat);
    memset(pFormat, 0, sizeof(*pFormat));

    pFormat->formatType = SL_DATAFORMAT_PCM;
    // Only support 2 channels
    // For channelMask, refer to wilhelm/src/android/channels.c for details
    if (pSampleInfo_->numChannels <= 1) {
        pFormat->numChannels = 1;
        pFormat->channelMask = SL_SPEAKER_FRONT_LEFT;
    } else {
        pFormat->numChannels = 2;
        pFormat->channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    pFormat->sampleRate = pSampleInfo_->sampleRate;

    pFormat->endianness = SL_BYTEORDER_LITTLEENDIAN;
    pFormat->bitsPerSample = pSampleInfo_->pcmFormat;
    pFormat->containerSize = pSampleInfo_->pcmFormat;

    // fixup for android extended representations...
    pFormat->representation = pSampleInfo_->representation;
    switch (pFormat->representation) {
        case SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_8;
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;  // supports 16, 24, and 32
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case SL_ANDROID_PCM_REPRESENTATION_FLOAT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_32;
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_32;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case 0:
            break;
        default:
            assert(0);
    }
}

static void createSLEngine(int sampleRate, int framesPerBuf, int numChannels)
{
    LOG_DEBUG(" == createSLEngine == ");
    LOG_DEBUG(" SampleRate: %d, SampleBufferSize: %d, numChannels: %d",
              sampleRate, framesPerBuf, numChannels);

    SLresult result;
    memset(&engine, 0, sizeof(engine));

    engine.fastPathSampleRate = (SLmilliHertz)(sampleRate) * 1000;
    engine.fastPathFramesPerBuf = (uint32_t)(framesPerBuf);
    engine.sampleChannels = (uint16_t)numChannels;
    engine.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;

    memset(&sampleFormat, 0, sizeof(sampleFormat));
    sampleFormat.pcmFormat = (uint16_t)engine.bitsPerSample;
    sampleFormat.framesPerBuf = engine.fastPathFramesPerBuf;

    sampleFormat.representation = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
    sampleFormat.numChannels = (uint16_t)engine.sampleChannels;
    sampleFormat.sampleRate = engine.fastPathSampleRate;

    // create SL engine
    result = slCreateEngine(&engine.slEngineObj, 0, NULL, 0, NULL, NULL);
    SLASSERT(result);

    result = (*engine.slEngineObj)->Realize(engine.slEngineObj, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    result = (*engine.slEngineObj)->GetInterface(engine.slEngineObj, SL_IID_ENGINE, &engine.slEngineItf);
    SLASSERT(result);
}

static void deleteSLEngine()
{
    LOG_DEBUG(" == deleteSLEngine ==");

    if (engine.slEngineObj != NULL) {
        (*engine.slEngineObj)->Destroy(engine.slEngineObj);
        engine.slEngineObj = NULL;
        engine.slEngineItf = NULL;
    }
}

static void createSLPlayer(SampleFormat *sampleFormat, SLEngineItf slEngine)
{
    LOG_DEBUG(" == createSLBufferQueueAudioPlayer ==");

    SLresult result;
    assert(sampleFormat);
    player.sampleInfo = *sampleFormat;

    // create and realize the output mix
    result = (*slEngine)->CreateOutputMix(slEngine, &player.outputMixObjectItf, 0, NULL, NULL);
    SLASSERT(result);
    result = (*player.outputMixObjectItf)->Realize(player.outputMixObjectItf, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, BUFFER_QUEUE_LEN};

    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, &player.sampleInfo);
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, player.outputMixObjectItf};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create fast path audio player: SL_IID_BUFFERQUEUE and SL_IID_VOLUME
    // and other non-signal processing interfaces are ok.
    SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioPlayer(slEngine, &player.playerObjectItf, &audioSrc, &audioSnk,
                                            sizeof(ids) / sizeof(ids[0]), ids, req);
    SLASSERT(result);

    // realize the player
    result = (*player.playerObjectItf)->Realize(player.playerObjectItf, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the play interface
    result = (*player.playerObjectItf)->GetInterface(player.playerObjectItf, SL_IID_PLAY,
                                                     &player.playItf);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*player.playerObjectItf)->GetInterface(player.playerObjectItf, SL_IID_BUFFERQUEUE,
                                                     &player.playBufferQueueItf);
    SLASSERT(result);

    // register callback on the buffer queue
    result = (*player.playBufferQueueItf)->RegisterCallback(player.playBufferQueueItf,
                                                            bufferQueuePlayerCallback, NULL);
    SLASSERT(result);
}

static void deleteSLPlayer()
{
    LOG_DEBUG(" == deleteSLPlayer ==");

    // destroy buffer queue audio player object, and invalidate all associated
    // interfaces
    if (player.playerObjectItf != NULL) {
        (*player.playerObjectItf)->Destroy(player.playerObjectItf);
        player.playerObjectItf = NULL;
        player.playItf = NULL;
        player.playBufferQueueItf = NULL;
    }

    // free buffer
    free(player.buffer);
    player.buffer = NULL;

    // destroy output mix object, and invalidate all associated interfaces
    if (player.outputMixObjectItf) {
        (*player.outputMixObjectItf)->Destroy(player.outputMixObjectItf);
        player.outputMixObjectItf = NULL;
    }
}

static void shutdownAudioPlayer()
{
    LOG_DEBUG(" == shutdownAudioPlayer ==");

    // make sure the audio player was created
    if (NULL != player.playItf) {
        SLresult result = (*player.playItf)->SetPlayState(player.playItf, SL_PLAYSTATE_STOPPED);
        SLASSERT(result);
    }

    deleteSLPlayer();
    deleteSLEngine();
}

void playAudioPlayer(const char* audioFilePath, int sampleRate, int bufSize)
{
    LOG_DEBUG(" == playAudioPlayer ==");

    file = fopen(audioFilePath, "r");

    // ToDo: Hardcoded number of channel for now
    // createSLEngine(48000, 1024, 2);  // (sampleRate, framesPerBuffer, numChannels)
    createSLEngine(sampleRate, bufSize, 2);  // (sampleRate, framesPerBuffer, numChannels)
    createSLPlayer(&sampleFormat, engine.slEngineItf);

    SLresult result = (*player.playItf)->SetPlayState(player.playItf, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);

    // buffer size (2 channels)
    player.bufferSize = sampleFormat.framesPerBuf * 2;
    player.buffer = (short *)calloc(player.bufferSize, sizeof(short));
    (*player.playBufferQueueItf)->Enqueue(player.playBufferQueueItf, player.buffer,
                                          player.bufferSize*sizeof(short));

    // ToDo: need an event loop and handle EOF
    // usleep(10000000);

    isEOF = false;
    while(1)
    {
        if(isEOF) {
            break;
        }
    }

    shutdownAudioPlayer();
}

void stopAudioPlayer()
{
    LOG_DEBUG(" == stopAudioPlayer ==");
    isEOF = true;
}
