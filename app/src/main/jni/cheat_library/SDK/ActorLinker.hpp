struct S_ActorLinker_API {

    uintptr_t myTransform;

    CGameObject *(*get_gameObject)(CActorLinker *, MethodInfo *);

    bool (*IsSelfCamp)(CActorLinker *, CActorLinker *,MethodInfo * );
};
extern S_ActorLinker_API ActorLinker_API;

class CActorLinker {
public:

    CGameObject *GetGameObject() {
        return ActorLinker_API.get_gameObject(this, nullptr);
    }

    CTransform *GetTransform() {
        return Il2Cpp::GetField<CTransform *>(this, ActorLinker_API.myTransform);
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
