//
// Created by Joseph Chen
// jocodoma@gmail.com
//

#ifndef ANDROIDNATIVEAUDIOPLAYER_JNIAPI_H
#define ANDROIDNATIVEAUDIOPLAYER_JNIAPI_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jstring JNICALL
Java_com_studio_jocodoma_nativeaudioplayer_MainActivity_stringFromJNI(
        JNIEnv* env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_studio_jocodoma_nativeaudioplayer_MainActivity_playAudioPlayer(
        JNIEnv* env, jobject thiz, jstring filePath, jint sampleRate, jint bufferSize);

JNIEXPORT void JNICALL
Java_com_studio_jocodoma_nativeaudioplayer_MainActivity_stopAudioPlayer(
        JNIEnv* env, jobject thiz);


#ifdef __cplusplus
}
#endif

#endif //ANDROIDNATIVEAUDIOPLAYER_JNIAPI_H
