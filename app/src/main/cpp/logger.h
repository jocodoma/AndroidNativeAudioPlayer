//
// Created by Joseph Chen
// jocodoma@gmail.com
//

#ifndef ANDROIDNATIVEAUDIOPLAYER_LOGGER_H
#define ANDROIDNATIVEAUDIOPLAYER_LOGGER_H

#include <android/log.h>

// log level 3 ~ 6
#define LOG_DEBUG(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_INFO(...)  ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOG_WARN(...)  ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOG_ERROR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#endif //ANDROIDNATIVEAUDIOPLAYER_LOGGER_H
