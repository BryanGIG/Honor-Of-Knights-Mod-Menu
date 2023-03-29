#include "API.h"
#include <locale>
#include <codecvt>
#include <unistd.h>
#include <obfuscate.h>
#include <Logger.h>
#include "utf8-cpp/utf8.h"


#define setPointer(func) auto func ##_addr = getExportFunction(handle, #func); \
if (!func ##_addr) return -1;\
func = reinterpret_cast<decltype(func)>(func ##_addr)

namespace {
    const void *(*il2cpp_assembly_get_image)(const void *assembly);

    void *(*il2cpp_domain_get)();

    void **(*il2cpp_domain_get_assemblies)(const void *domain, size_t *size);

    const char *(*il2cpp_image_get_name)(void *image);

    void *(*il2cpp_class_from_name)(const void *image, const char *namespaze, const char *name);

    void *(*il2cpp_class_get_field_from_name)(void *klass, const char *name);

    void *(*il2cpp_class_get_method_from_name)(void *klass, const char *name, int argsCount);

    size_t (*il2cpp_field_get_offset)(void *field);

    void (*il2cpp_field_static_get_value)(void *field, void *value);

    void (*il2cpp_field_static_set_value)(void *field, void *value);

    void *(*il2cpp_array_new)(void *elementTypeInfo, size_t length);

    int32_t(*il2cpp_string_length)(Il2CppString* str);

    Il2CppChar *(*il2cpp_string_chars)( Il2CppString *str);

    char *(*il2cpp_type_get_name)(void *type);

    void* (*il2cpp_method_get_param)(void *method, uint32_t index);

    void* (*il2cpp_class_get_methods)(void *klass, void* *iter);

    const char* (*il2cpp_method_get_name)(void *method);

    void *(*il2cpp_object_new)(void *);

    void *(*il2cpp_thread_attach)(void *domain);
}
void *getExportFunction(void *handle, const char *name) {
    void *fn = dlsym(handle, name);
    if (fn) {
        return fn;
    }

    LOGI("%s: %p %s", __FUNCTION__, handle, name);
    return nullptr;
}

namespace Il2Cpp {
    bool IsReady = false;

    int Attach(const char *lib) {
        void *handle = dlopen(lib, RTLD_LAZY);
        if (!handle) {
            LOGE("Failed to load library: %s", dlerror());
            return -1;
        }
        return Attach(handle);
    }

    int Attach(void *handle) {
        setPointer(il2cpp_assembly_get_image);
        setPointer(il2cpp_thread_attach);
        setPointer(il2cpp_domain_get);
        setPointer(il2cpp_domain_get_assemblies);
        setPointer(il2cpp_image_get_name);
        setPointer(il2cpp_class_from_name);
        setPointer(il2cpp_class_get_field_from_name);
        setPointer(il2cpp_class_get_method_from_name);
        setPointer(il2cpp_field_get_offset);
        setPointer(il2cpp_field_static_get_value);
        setPointer(il2cpp_field_static_set_value);
        setPointer(il2cpp_array_new);
        setPointer(il2cpp_string_chars);
        setPointer(il2cpp_string_length);
        setPointer(il2cpp_type_get_name);
        setPointer(il2cpp_method_get_param);
        setPointer(il2cpp_class_get_methods);
        setPointer(il2cpp_method_get_name);
        setPointer(il2cpp_object_new);
        return 0;
    }

    void AttachThread() {
        il2cpp_thread_attach(il2cpp_domain_get());
    }

    void *GetImageByName(const char *image) {
        size_t size;
        void **assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &size);
        for (int i = 0; i < size; ++i) {
            void *img = (void *) il2cpp_assembly_get_image(assemblies[i]);
            const char *img_name = il2cpp_image_get_name(img);
            if (strcmp(img_name, image) == 0) {
                return img;
            }
        }
        return nullptr;
    }

    void GetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *output) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return;
        }
        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s for field %s!", clazz, name);
            return;
        }

        void *field = il2cpp_class_get_field_from_name(klass, name);
        if (!field) {
            LOGI("Can't find field %s in class %s!", name, clazz);
            return;
        }

        il2cpp_field_static_get_value(field, output);
    }

    void SetStaticFieldValue(const char *image, const char *namespaze, const char *clazz, const char *name, void *value) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return;
        }
        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s for field %s!", clazz, name);
            return;
        }

        void *field = il2cpp_class_get_field_from_name(klass, name);
        if (!field) {
            LOGI("Can't find field %s in class %s!", name, clazz);
            return;
        }

        il2cpp_field_static_set_value(field, value);
    }

    void *GetClassType(const char *image, const char *namespaze, const char *clazz) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return nullptr;
        }

        void *klass = il2cpp_class_from_name(img, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s!", clazz);
            return nullptr;
        }
        return klass;
    }

    void *CreateClassInstance(const char *image, const char *namespaze, const char *clazz) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return nullptr;
        }

        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s!", clazz);
            return nullptr;
        }

        void *obj = il2cpp_object_new(klass);
        if (!obj) {
            LOGI("Can't create object for %s", clazz);
            return nullptr;
        }

        return obj;
    }

    void *GetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, int argsCount) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return nullptr;
        }

        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s for method %s!", clazz, name);
            return nullptr;
        }

        void **method = (void **) il2cpp_class_get_method_from_name(klass, name, argsCount);
        if (!method) {
            LOGI("Can't find method %s in class %s!", name, clazz);
            return nullptr;
        }

        return *method;
    }

    void *GetMethodOffset(const char *image, const char *namespaze, const char *clazz, const char *name, char **args, int argsCount) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return nullptr;
        }

        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s for method %s!", clazz, name);
            return nullptr;
        }

        void *iter = nullptr;

        int score = 0;

        void **method = (void **) il2cpp_class_get_methods(klass, &iter);
        while (method) {
            const char *fname = il2cpp_method_get_name(method);
            if (strcmp(fname, name) == 0) {
                for (int i = 0; i < argsCount; i++) {
                    void *arg = il2cpp_method_get_param(method, i);
                    if (arg) {
                        const char *tname = il2cpp_type_get_name(arg);
                        if (strcmp(tname, args[i]) == 0) {
                            score++;
                        } else {
                            LOGI("Argument at index %d didn't matched requested argument!\r\n\tRequested: %s\r\n\tActual: %s\r\nSkipping function...",
                                 i, args[i], tname);
                            score = 0;
                            goto skip;
                        }
                    }
                }
            }
            skip:

            if (score == argsCount) {
                LOGI("Found matched function!");
                return *method;
            }

            method = (void **) il2cpp_class_get_methods(klass, &iter);
        }
        LOGI("Cannot find function %s in class %s!", name, clazz);
        return nullptr;
    }

    const char *GetMethodName(void *method) {
        return il2cpp_method_get_name(method);
    }

    uintptr_t GetFieldOffset(void *clazz, const char *name) {
        void *field = il2cpp_class_get_field_from_name(clazz, name);
        if (!field) {
            LOGI("Can't find field %s!", name);
            return -1;
        }
        return il2cpp_field_get_offset(field);
    }

    size_t GetFieldOffset(const char *image, const char *namespaze, const char *clazz, const char *name) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return -1;
        }
        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s for field %s!", clazz, name);
            return -1;
        }

        void *field = il2cpp_class_get_field_from_name(klass, name);
        if (!field) {
            LOGI("Can't find field %s in class %s!", name, clazz);
            return -1;
        }
        return il2cpp_field_get_offset(field);
    }

    bool IsAssembliesLoaded() {
        size_t size;
        void **assemblies = il2cpp_domain_get_assemblies(il2cpp_domain_get(), &size);

        return size != 0 && assemblies != nullptr;
    }

    void *CreateArray(const char *image, const char *namespaze, const char *clazz, size_t length) {
        void *img = GetImageByName(image);
        if (!img) {
            LOGI("Can't find image %s!", image);
            return nullptr;
        }
        void *klass = GetClassType(image, namespaze, clazz);
        if (!klass) {
            LOGI("Can't find class %s!", clazz);
            return nullptr;
        }

        return il2cpp_array_new(klass, length);
    }

    std::string GetString(Il2CppString *string) {
        if (!string)
            return "ERROR: String is null";

        auto chars = il2cpp_string_chars(string);
        auto length = il2cpp_string_length(string);
        std::string utf8String;
        utf8String.reserve(length);
        utf8::unchecked::utf16to8(chars, chars + length, std::back_inserter(utf8String));
        return utf8String;
    }
}