#pragma once

#ifndef API_H
#define API_H

#include <stdio.h>
#include <dlfcn.h>
#include <cstring>
#include <string>
#include <vector>
#include <setjmp.h>
#include <signal.h>
#include "il2cpp.h"
using namespace std;

typedef uint16_t Il2CppChar;
struct Il2CppString {
    Il2CppObject object;
    int32_t length;
    Il2CppChar chars[32];
};

namespace Il2Cpp {
    extern bool IsReady; //the Il2Cpp API is ready or not

    int Attach(const char *lib = "libil2cpp.so");

    int Attach(void *handle = nullptr);

    void *GetImageByName(const char *image);

    void *GetClassType(const char *image, const char *namespaze, const char *clazz);

    void *CreateArray(const char *image, const char *namespaze, const char *clazz, size_t length);

    void *GetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, int argsCount = 0);

    void *GetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, char **args, int argsCount = 0);

    const char *GetMethodName(void* method);

    size_t GetFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name);

    void GetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *output);

    void SetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void* value);

    void *CreateClassInstance(const char *image, const char *namespaze, const char *clazz);

    bool IsAssembliesLoaded();

    void *CreateString(const char *str);

    /**
    * Thanks to geokar2006
    */
    template<typename T, typename ... Args>
    inline bool SafeCall(T(*func)(Args...), T &out, Args ...args) {
        static jmp_buf jump;
        static sighandler_t handler = [](int) { longjmp(jump, 1); };
        volatile sighandler_t old_sig = signal(11, handler);
        bool result = true;
        if (!setjmp (jump)) {
            out = func(args...);
        } else {
            result = false;
        }
        signal(11, old_sig);
        return result;
    }

    template <typename T>
    inline T GetMethod(uintptr_t realOffset) {
        return (T) (realOffset);
    }

    template <typename T>
    inline T GetMethod(const char *image, const char *namespaze, const char *clazz, const char *name, int argsCount) {
        auto realOffset = Il2Cpp::GetMethodOffset(image, namespaze, clazz, name, argsCount);
        return (T) (realOffset);
    }

    template <typename T>
    inline T GetField(void* obj, std::ptrdiff_t Offset) {
        return *(T *) ((std::uint8_t *) obj + Offset);
    }

    template <typename T>
    inline void SetField(void* obj, std::ptrdiff_t Offset, T val) {
        *(T *) ((std::uint8_t *) obj + Offset) = val;
    }

    string GetString(Il2CppString *string);

    uintptr_t GetFieldOffset(void *clazz, const char *name);

    void AttachThread();
}

#endif //API_H
