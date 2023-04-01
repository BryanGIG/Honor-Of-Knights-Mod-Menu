struct S_ValueLinkerComponent_API {
    uintptr_t actorExp;
    uintptr_t actorMaxExp;
    uintptr_t actorHp;
    uintptr_t actorHpTotal;
    uintptr_t actorProtectMax;
    uintptr_t actorProtect;
};

extern S_ValueLinkerComponent_API ValueLinkerComponent_API;

class CValueLinkerComponent {
public:
    int GetExp() {
        return Il2Cpp::GetField<int>(this, ValueLinkerComponent_API.actorExp);
    }

    int GetMaxExp() {
        return Il2Cpp::GetField<int>(this, ValueLinkerComponent_API.actorMaxExp);
    }

    int GetHp() {
        return Il2Cpp::GetField<int>(this, ValueLinkerComponent_API.actorHp);
    }

    int GetMaxHp() {
        return Il2Cpp::GetField<int>(this, ValueLinkerComponent_API.actorHpTotal);
    }

    int GetMaxArmor() {
        return Il2Cpp::GetField<int>(this, ValueLinkerComponent_API.actorProtectMax);
    }

    int GetArmor() {
        return Il2Cpp::GetField<int>(this, ValueLinkerComponent_API.actorProtect);
    }
};

namespace ValueLinkerComponent {

    const char *getAssembly();

    const char *getNamespace();

    const char *getClass();

    void Initialize();

} // ValueLinkerComponent
