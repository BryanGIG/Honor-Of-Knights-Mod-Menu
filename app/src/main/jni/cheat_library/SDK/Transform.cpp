#include "Includes.hpp"

S_Transform_API Transform_API;

namespace Transform {
    const char *getAssembly() {
        return OBFUSCATE("UnityEngine.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("UnityEngine");
    }

    const char *getClass() {
        return OBFUSCATE("Transform");
    }

    void Initialize() {
        setPointer(Transform_API.get_rotation, "get_rotation", 0);
        setPointer(Transform_API.get_position, "get_position", 0);
    }
}