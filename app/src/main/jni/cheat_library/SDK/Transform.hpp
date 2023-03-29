struct S_Transform_API {

    Vector3 (*get_position)(CTransform *, MethodInfo *);

    Quaternion (*get_rotation)(CTransform *, MethodInfo *);

};

extern S_Transform_API Transform_API;

class CTransform : public CComponent {
public:

    Vector3 GetPosition() {
        return Transform_API.get_position(this, nullptr);
    }

    Quaternion GetRotation() {
        return Transform_API.get_rotation(this, nullptr);
    }
};

namespace Transform {
    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();
}