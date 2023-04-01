#include "Includes.hpp"

S_ValueLinkerComponent_API ValueLinkerComponent_API;

namespace ValueLinkerComponent {

    const char *getAssembly() {
        return OBFUSCATE("Scripts.GameCore.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("Assets.Scripts.GameLogic");
    }

    const char *getClass() {
        return OBFUSCATE("ValueLinkerComponent");
    }

    void Initialize() {
        ValueLinkerComponent_API.actorExp = getFieldOffset("actorExp");
        ValueLinkerComponent_API.actorMaxExp = getFieldOffset("actorMaxExp");
        ValueLinkerComponent_API.actorHp = getFieldOffset("actorHp");;
        ValueLinkerComponent_API.actorHpTotal = getFieldOffset("actorHpTotal");;
        ValueLinkerComponent_API.actorProtectMax = getFieldOffset("actorProtectMax");;
        ValueLinkerComponent_API.actorProtect = getFieldOffset("actorProtect");;
    }
} // ValueLinkerComponent