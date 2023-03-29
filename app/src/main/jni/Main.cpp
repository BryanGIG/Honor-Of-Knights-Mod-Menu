#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "cheat_main/HookManager.h"

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *) {
    auto &ins = cheat::main::HookManager::GetInstance();
    ins.StartThread();
    return JNI_VERSION_1_6;
}