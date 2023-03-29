#pragma once

#include <thread>
#include <SDK/Includes.hpp>
#include <Utils.h>
#include "events.h"
#include <MemUtil.h>

#define targetLibName OBFUSCATE("libil2cpp.so")
namespace cheat {
    namespace main {
        class HookManager {
        public:
            static HookManager &GetInstance();

            void StartThread();

        private:
            HookManager();
        };
    }
}