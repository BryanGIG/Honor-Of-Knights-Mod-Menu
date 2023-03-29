#pragma once

struct S_Camera_API {
    CCamera *(*get_main)(void *, MethodInfo *method);

    Matrix4x4 (*get_projectionMatrix)(CCamera *, MethodInfo *method);

    Matrix4x4 (*get_worldToCameraMatrix)(CCamera *, MethodInfo *method);
};

extern S_Camera_API Camera_API;

class CCamera {
public:
    Matrix4x4 getProjectionMatrix() {
        return Camera_API.get_projectionMatrix(this, nullptr);
    }

    Matrix4x4 getWorldToCameraMatrix() {
        return Camera_API.get_worldToCameraMatrix(this, nullptr);
    }

};

namespace Camera {
    inline Matrix4x4 _vMatrix;

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();

    CCamera *get_main();

    bool WorldToScreen(Vector3 from, Vector2 *to);
}