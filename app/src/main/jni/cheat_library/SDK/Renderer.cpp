#include "Includes.hpp"

S_Renderer_API Renderer_API;

namespace Renderer {

    const char *getAssembly() {
        return OBFUSCATE("UnityEngine.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("UnityEngine");
    }

    const char *getClass() {
        return OBFUSCATE("Renderer");
    }

    void Initialize() {
        setPointer(Renderer_API.get_bounds, "get_bounds", 0);
    }

} // Renderer