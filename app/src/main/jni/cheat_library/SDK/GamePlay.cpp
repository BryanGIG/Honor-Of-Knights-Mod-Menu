#include "Includes.hpp"

S_GamePlay_API GamePlay_API;

namespace GamePlay {
    const char *getAssembly() {
        return OBFUSCATE("Scripts.GameCore.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("Assets.Scripts.GameLogic");
    }

    const char *getClass() {
        return OBFUSCATE("GamePlay");
    }

    void Initialize() {
        setPointer(GamePlay_API.get_Instance, "GetInstance", 0);
        setPointer(GamePlay_API.HasInstance, "HasInstance", 0);
        setPointer(GamePlay_API.Update, "Update", 0);
        setPointer(GamePlay_API.LateUpdate, "LateUpdate", 0);
    }

    CGamePlay *GetInstance() {
        if (GamePlay_API.HasInstance(nullptr, nullptr)) {
            return GamePlay_API.get_Instance(nullptr, nullptr);
        } else {
            return nullptr;
        }
    }
}