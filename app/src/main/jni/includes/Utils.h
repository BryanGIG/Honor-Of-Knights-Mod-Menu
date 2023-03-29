#pragma once

#ifndef UTILS
#define UTILS

#include <jni.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include "Logger.h"

typedef unsigned long DWORD;
extern uintptr_t libBase;

extern bool libLoaded;

DWORD findLibrary(const char *library);

DWORD getAbsoluteAddress(const char *libraryName, DWORD relativeAddr);

bool isLibraryLoaded(const char *libraryName);

uintptr_t string2Offset(const char *c);

namespace ToastLength {
    inline const int LENGTH_LONG = 1;
    inline const int LENGTH_SHORT = 0;
}

#endif