#include "Includes.hpp"

S_GameObject_API GameObject_API;

namespace GameObject {
    const char *getAssembly() {
        return OBFUSCATE("UnityEngine.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("UnityEngine");
    }

    const char *getClass() {
        return OBFUSCATE("GameObject");
    }

    void Initialize() {
        setPointer(GameObject_API.get_transform, "get_transform", 0);
        setPointer(GameObject_API.get_tag, "get_tag", 0);
        setPointer(GameObject_API.get_active, "get_active", 0);
    }
}