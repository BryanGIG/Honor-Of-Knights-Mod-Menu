#include "MemUtil.h"
#include "Logger.h"
#include <libgen.h>

#define _SYS_PAGE_SIZE_ (sysconf(_SC_PAGE_SIZE))

#define _PAGE_START_OF_(x)    ((uintptr_t)x & ~(uintptr_t)(_SYS_PAGE_SIZE_ - 1))
#define _PAGE_END_OF_(x, len) (_PAGE_START_OF_((uintptr_t)x + len - 1))
#define _PAGE_LEN_OF_(x, len) (_PAGE_END_OF_(x, len) - _PAGE_START_OF_(x) + _SYS_PAGE_SIZE_)
#define _PAGE_OFFSET_OF_(x)   ((uintptr_t)x - _PAGE_START_OF_(x))

#define _PROT_RWX_ (PROT_READ | PROT_WRITE | PROT_EXEC)
#define _PROT_RX_  (PROT_READ | PROT_EXEC)

#define INRANGE(x, a, b)        (x >= a && x <= b)
#define getBits(x)              (INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte(x)              (getBits(x[0]) << 4 | getBits(x[1]))

namespace MemUtil {
    uintptr_t GetBaseAddress(const char *name) {
        uintptr_t base = 0;
        char line[512];

        FILE *f = fopen("/proc/self/maps", "r");

        if (!f) {
            return 0;
        }

        while (fgets(line, sizeof line, f)) {
            uintptr_t tmpBase;
            char tmpName[256];
            if (sscanf(line, "%" PRIXPTR "-%*" PRIXPTR " %*s %*s %*s %*s %s", &tmpBase, tmpName) >
                0) {
                if (!strcmp(basename(tmpName), name)) {
                    base = tmpBase;
                    break;
                }
            }
        }

        fclose(f);
        return base;
    }

    uintptr_t GetEndAddress(const char *name) {
        uintptr_t end = 0;
        char line[512];

        FILE *f = fopen("/proc/self/maps", "r");

        if (!f) {
            return 0;
        }

        bool found = false;
        while (fgets(line, sizeof line, f)) {
            uintptr_t tmpEnd;
            char tmpName[256];
            if (sscanf(line, "%*" PRIXPTR "-%" PRIXPTR " %*s %*s %*s %*s %s", &tmpEnd, tmpName) >
                0) {
                if (!strcmp(basename(tmpName), name)) {
                    if (!found) {
                        found = true;
                    }
                } else {
                    if (found) {
                        end = tmpEnd;
                        break;
                    }
                }
            }
        }

        fclose(f);
        return end;
    }

    bool ReadAddr(void *addr, void *buffer, size_t length) {
        unsigned long page_size = sysconf(_SC_PAGESIZE);
        unsigned long size = page_size * sizeof(uintptr_t);
        return mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size),
                        (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 &&
               memcpy(buffer, addr, length) != 0;
    }

    bool WriteAddr(void *addr, void *buffer, size_t length) {
        unsigned long page_size = sysconf(_SC_PAGESIZE);
        unsigned long size = page_size * sizeof(uintptr_t);
        bool ok = mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size),
                           (size_t) size, PROT_EXEC | PROT_READ | PROT_WRITE) == 0 &&
                  memcpy(addr, buffer, length) != 0;
        mprotect((void *) ((uintptr_t) addr - ((uintptr_t) addr % page_size) - page_size),
                 (size_t) size, PROT_EXEC | PROT_READ);
        return ok;
    }

    bool SetProtRW(void *addr) {
        uintptr_t pageStart = _PAGE_START_OF_(addr);
        uintptr_t pageLen = _PAGE_LEN_OF_(addr, 4);
        return (mprotect(reinterpret_cast<void *>(pageStart), pageLen, _PROT_RX_) != -1);
    }

    bool SetProtRWX(void *addr) {
        uintptr_t pageStart = _PAGE_START_OF_(addr);
        uintptr_t pageLen = _PAGE_LEN_OF_(addr, 4);
        return (mprotect(reinterpret_cast<void *>(pageStart), pageLen, _PROT_RWX_) != -1);
    }

    bool MemoryProtect(void *address, bool restore) {
        std::uintptr_t pageStart = _PAGE_START_OF_(address);
        std::size_t pageLen = _SYS_PAGE_SIZE_ * sizeof(uintptr_t);

        if (restore)
            return mprotect(reinterpret_cast<void *>(pageStart), pageLen, PROT_EXEC | PROT_READ) !=
                   -1;

        return mprotect(reinterpret_cast<void *>(pageStart), pageLen,
                        PROT_EXEC | PROT_READ | PROT_WRITE) != -1;
    }

    void WriteAddress(uintptr_t addr, uint64_t replace) {
        MemoryProtect((void *) addr, false);
        *(uint64_t *) (addr) = replace;
        MemoryProtect((void *) addr, true);
    }

    void *InlineHooker(void *target, void *hooker) {
        SetProtRWX(target);
        dobby_dummy_func_t origin_call;
        if (DobbyHook(target, (dobby_dummy_func_t) hooker, &origin_call) == RS_SUCCESS) {
            return (void *) origin_call;
        } else {
            LOGE("Failed Hook %p", target);
            return nullptr;
        }
    }

    bool InlineUnhooker(void *func) {
        return DobbyDestroy(func) == RT_SUCCESS;
    }

}
