#pragma once

struct S_GameObject_API {
    CTransform *(*get_transform)(CGameObject *, MethodInfo *method);

    Il2CppString *(*get_tag)(CGameObject *, MethodInfo *method);

    bool (*get_active)(CGameObject *, MethodInfo *method);
};

extern S_GameObject_API GameObject_API;

class CGameObject : public CObject {
public:

    bool IsActive() {
        return GameObject_API.get_active(this, nullptr);
    }

    CTransform *GetTransform() {
        return GameObject_API.get_transform(this, nullptr);
    }

    std::string GetTag() {
        auto name = GameObject_API.get_tag(this, nullptr);
        if (!name)
            return "";

        return Il2Cpp::GetString(name);
    }
};

namespace GameObject {
    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();
}