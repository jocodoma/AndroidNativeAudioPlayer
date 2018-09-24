//
// Created by Joseph Chen
// jocodoma@gmail.com
//

#ifndef ANDROIDNATIVEAUDIOPLAYER_AUDIOPLAYER_H
#define ANDROIDNATIVEAUDIOPLAYER_AUDIOPLAYER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#ifdef __cplusplus
extern "C" {
#endif

void playAudioPlayer(const char* audioFilePath, int sampleRate, int bufSize);
void stopAudioPlayer();

#ifdef __cplusplus
}
#endif

#endif //ANDROIDNATIVEAUDIOPLAYER_AUDIOPLAYER_H
