#include "Includes.hpp"

S_ActorCreators_API ActorCreators_API;

namespace ActorCreators {
    const char *getAssembly() {
        return OBFUSCATE("Scripts.GameCore.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("Assets.Scripts.GameLogic");
    }

    const char *getClass() {
        return OBFUSCATE("ActorCreators");
    }

    void Initialize() {
        setPointer(ActorCreators_API.CreateHeroComponent, "CreateHeroComponent", 1);
        setPointer(ActorCreators_API.DestoryHeroComponent, "DestoryHeroComponent", 1);
    }

} // ActorCreators