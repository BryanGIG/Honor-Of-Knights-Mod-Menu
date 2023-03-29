struct S_Object_API {
    Il2CppString *(*get_name)(CObject *, MethodInfo *);
};

extern S_Object_API Object_API;

class CObject {
public:
    std::string GetName() {
        return Il2Cpp::GetString(Object_API.get_name(this, nullptr));
    }
};

namespace Object {
    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();
}