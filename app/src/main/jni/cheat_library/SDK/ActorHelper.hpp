struct S_ActorHelper_API {
    void *(*GetHostCtrlActorProxy)(void *, MethodInfo *);
};
extern S_ActorHelper_API ActorHelper_API;

class CActorHelper {
public:
};

namespace ActorHelper {
    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();

    CActorLinker *GetLocalActor();
} // ActorHelper