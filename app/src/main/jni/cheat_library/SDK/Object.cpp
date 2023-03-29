#include "Includes.hpp"

S_Object_API Object_API;

namespace Object {
    const char *getAssembly() {
        return OBFUSCATE("UnityEngine.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("UnityEngine");
    }

    const char *getClass() {
        return OBFUSCATE("Object");
    }

    void Initialize() {
        setPointer(Object_API.get_name, "get_name", 0);
    }
}