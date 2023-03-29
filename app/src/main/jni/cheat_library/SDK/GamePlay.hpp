
struct S_GamePlay_API {


    bool (*HasInstance)(void *, MethodInfo *);
    CGamePlay *(*get_Instance)(void *, MethodInfo *);

    void *Update;
    void *LateUpdate;
};

extern S_GamePlay_API GamePlay_API;

class CGamePlay {
public:

};

namespace GamePlay {

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();

    CGamePlay *GetInstance();
}