struct S_Renderer_API {
    Bounds (*get_bounds)(CRenderer *, MethodInfo *);
};

extern S_Renderer_API Renderer_API;

class CRenderer {
public:
    Bounds GetBounds() {
        return Renderer_API.get_bounds(this, nullptr);
    }
};

namespace Renderer {

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();

} // Renderer
