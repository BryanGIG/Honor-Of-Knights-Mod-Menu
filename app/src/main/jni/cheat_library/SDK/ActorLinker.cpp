#include "Includes.hpp"

S_ActorLinker_API ActorLinker_API;

namespace ActorLinker {

    const char *getAssembly() {
        return OBFUSCATE("Scripts.GameCore.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("Assets.Scripts.GameLogic");
    }

    const char *getClass() {
        return OBFUSCATE("ActorLinker");
    }

    void Initialize() {
        ActorLinker_API.myRenderer = getFieldOffset("myRenderer");
        ActorLinker_API.myTransform = getFieldOffset("myTransform");
        ActorLinker_API.ValueComponent = getFieldOffset("ValueComponent");
        ActorLinker_API. position = getFieldOffset("position");
        ActorLinker_API. name = getFieldOffset("name");

        setPointer(ActorLinker_API.get_gameObject, "get_gameObject", 0);
        setPointer(ActorLinker_API.IsSelfCamp, "IsSelfCamp", 1);
    }


} // ActorLinker