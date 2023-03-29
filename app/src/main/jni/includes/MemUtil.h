#pragma once

#ifndef HOOK_DEF

#include <dobby.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cinttypes>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <KittyMemory/KittyMemory.h>
#include <KittyMemory/MemoryPatch.h>
#include <KittyMemory/KittyUtils.h>

using KittyMemory::ProcMap;

#define HOOK(addr, func)orig_##func = decltype(orig_##func)(MemUtil::InlineHooker(addr, (void *) new_##func))

#define HOOK_DEF(ret, func, ...) \
  ret (*orig_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)

namespace MemUtil {

    uintptr_t GetBaseAddress(const char *name);

    uintptr_t GetEndAddress(const char *name);

    bool ReadAddr(void *addr, void *buffer, size_t length);

    bool WriteAddr(void *addr, void *buffer, size_t length);

    bool SetProtRW(void *addr);

    bool SetProtRWX(void *addr);

    bool MemoryProtect(void *address, bool restore);

    void WriteAddress(uintptr_t addr, uint64_t replace);

    void *InlineHooker(void *target, void *hooker);

    bool InlineUnhooker(void *func);

    auto IsPtrValid = [](auto x) -> bool {
        int nullfd = open("/dev/random", (int)"\1"[0]);
        bool ok = write(nullfd, (void *) x, sizeof(x)) >= 0;
        close(nullfd);
        return ok;
    };
}
#endif