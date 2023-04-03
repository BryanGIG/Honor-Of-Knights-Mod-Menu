struct S_ActorLinker_API {

    uintptr_t position;

    uintptr_t name;

    uintptr_t myRenderer;

    uintptr_t myTransform;

    uintptr_t ValueComponent;

    CGameObject *(*get_gameObject)(CActorLinker *, MethodInfo *);

    bool (*IsSelfCamp)(CActorLinker *, CActorLinker *,MethodInfo * );

};
extern S_ActorLinker_API ActorLinker_API;

class CActorLinker {
public:
    CValueLinkerComponent *GetValueComponent() {
        return Il2Cpp::GetField<CValueLinkerComponent *>(this, ActorLinker_API.ValueComponent);
    }

    CRenderer *GetRenderer() {
        return Il2Cpp::GetField<CRenderer *>(this, ActorLinker_API.myRenderer);
    }

    CGameObject *GetGameObject() {
        return ActorLinker_API.get_gameObject(this, nullptr);
    }

    CTransform *GetTransform() {
        return Il2Cpp::GetField<CTransform *>(this, ActorLinker_API.myTransform);
    }

    std::string GetName() {
        auto str = Il2Cpp::GetField<Il2CppString *>(this, ActorLinker_API.name);
        return Il2Cpp::GetString(str);
    }

    Vector3 GetPosition() {
        return Il2Cpp::GetField<Vector3>(this, ActorLinker_API.position);
    }

    bool IsSelfCamp(CActorLinker *other) {
        return ActorLinker_API.IsSelfCamp(this, other, nullptr);
    }
};

namespace ActorLinker {

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();


} // ActorLinker
