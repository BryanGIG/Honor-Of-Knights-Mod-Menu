#include "Includes.hpp"

S_Component_API Component_API;

namespace Component {

    const char *getAssembly() {
        return OBFUSCATE("UnityEngine.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("UnityEngine");
    }

    const char *getClass() {
        return OBFUSCATE("Component");
    }

    void Initialize() {
        setPointer(Component_API.get_transform, "get_transform", 0);
        setPointer(Component_API.get_gameObject, "get_gameObject", 0);
        setPointer(Component_API.get_tag, "get_tag", 0);
    }
}