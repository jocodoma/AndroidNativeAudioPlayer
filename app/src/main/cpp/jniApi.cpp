//
// Created by Joseph Chen
// jocodoma@gmail.com
//

#include "jniApi.h"
#include "logger.h"
#include "audioPlayer.h"
#include <string>

#define LOG_TAG "[jniApi.c]"

jstring Java_com_studio_jocodoma_nativeaudioplayer_MainActivity_stringFromJNI(
        JNIEnv *env, jobject /* this */)
{
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

void Java_com_studio_jocodoma_nativeaudioplayer_MainActivity_playAudioPlayer(
        JNIEnv* env, jobject thiz, jstring filePath, jint sampleRate, jint bufSize)
{
    LOG_DEBUG(" == playAudioPlayer == ");

    const char *audioFilePath = env->GetStringUTFChars(filePath, 0);
    playAudioPlayer(audioFilePath, sampleRate, bufSize);
    env->ReleaseStringUTFChars(filePath, audioFilePath);
}

void Java_com_studio_jocodoma_nativeaudioplayer_MainActivity_stopAudioPlayer(
        JNIEnv* env, jobject thiz)
{
    LOG_DEBUG(" == stopAudioPlayer == ");
    stopAudioPlayer();
}

