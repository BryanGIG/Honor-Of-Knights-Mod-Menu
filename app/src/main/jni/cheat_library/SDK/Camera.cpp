
#include "Includes.hpp"

S_Camera_API Camera_API;

namespace Camera {
    const char *getAssembly() {
        return OBFUSCATE("UnityEngine.dll");
    }

    const char *getNamespace() {
        return OBFUSCATE("UnityEngine");
    }

    const char *getClass() {
        return OBFUSCATE("Camera");
    }

    void Initialize() {
        setPointer(Camera_API.get_main, "get_main", 0);
        setPointer(Camera_API.get_projectionMatrix, "get_projectionMatrix", 0);
        setPointer(Camera_API.get_worldToCameraMatrix, "get_worldToCameraMatrix", 0);
    }

    CCamera *get_main() {
        return Camera_API.get_main(nullptr, nullptr);
    }

    bool WorldToScreen(Vector3 from, Vector2 *to) {
        auto viewMatrix = _vMatrix.MultiplyPoint(from);
        auto screenPos = Vector3(
                viewMatrix.X + 1.0f,
                viewMatrix.Y + 1.0f,
                viewMatrix.Z + 1.0f) / 2.0f;
        *to = Vector2(screenPos.X * Screen::glWidth, Screen::glHeight - (screenPos.Y * Screen::glHeight));
        return viewMatrix != Vector3::Zero();
    }
}