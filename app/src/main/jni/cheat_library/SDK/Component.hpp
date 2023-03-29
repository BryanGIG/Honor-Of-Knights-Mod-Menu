struct S_Component_API {
    CTransform *(*get_transform)(CComponent *, MethodInfo *method);

    CGameObject *(*get_gameObject)(CComponent *, MethodInfo *method);

    Il2CppString *(*get_tag)(CComponent *, MethodInfo *method);
};

extern S_Component_API Component_API;

class CComponent : public CObject {
public:
    CTransform *GetTransform() {
        return Component_API.get_transform(this, nullptr);
    }

    CGameObject *GetGameObject() {
        return Component_API.get_gameObject(this, nullptr);
    }

    std::string GetTag() {
        auto tag = Component_API.get_tag(this, nullptr);
        return Il2Cpp::GetString(tag);
    }
};

namespace Component {
    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();
}