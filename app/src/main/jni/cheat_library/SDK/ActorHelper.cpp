#include "Includes.hpp"

S_ActorHelper_API ActorHelper_API;

namespace ActorHelper {

    const char *getAssembly() {
        return OBFUSCATE("Scripts.GameCore.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("Assets.Scripts.GameLogic");
    }

    const char *getClass() {
        return OBFUSCATE("ActorHelper");
    }

    void Initialize() {
        setPointer(ActorHelper_API.GetHostCtrlActorProxy, "GetHostCtrlActorProxy", 0);
    }

    CActorLinker *GetLocalActor() {
        return (CActorLinker *) ActorHelper_API.GetHostCtrlActorProxy(nullptr, nullptr);
    }
} // ActorHelper