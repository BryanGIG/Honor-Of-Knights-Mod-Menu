struct S_ActorCreators_API {
    void *CreateHeroComponent;
    void *DestoryHeroComponent;
};


extern S_ActorCreators_API ActorCreators_API;

class CActorCreators {
public:
};

namespace ActorCreators {

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();
} // ActorCreators
