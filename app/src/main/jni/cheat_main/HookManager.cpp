#include "HookManager.h"
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include "MThreadRenderer.h"

namespace cheat::main {
    static std::vector<CActorLinker *> actors;
    static bool isFirstInit;
    static bool isSurfaceChange = false;
    static bool isSurfaceDestroyed = false;
    static bool isPause = false;
    static EGLDisplay egl_Display;
    static EGLSurface egl_Surface;
    static EGLConfig egl_Config;
    static EGLContext egl_Context;
    static ANativeWindow *nativeWindow;

    HOOK_DEF(void, CreateHeroComponent, void *actorCreator, CActorLinker *actor, MethodInfo *method) {
        orig_CreateHeroComponent(actorCreator, actor, method);

        if (!actor)
            return;

        actors.push_back(actor);
    }

    HOOK_DEF(void, DestroyHeroComponent, void *actorCreator, CActorLinker *actor, MethodInfo *method) {
        orig_DestroyHeroComponent(actorCreator, actor, method);

        if (!actor)
            return;

        erase_if(actors, [&](const auto &actorit) {
            return actorit == actor;
        });
    }

    HOOK_DEF(void, GamePlay_LateUpdate, CGamePlay *ins, MethodInfo *method) {
        orig_GamePlay_LateUpdate(ins, method);

        auto camera = Camera::get_main();
        if (!camera)
            return;

        Camera::_vMatrix = camera->getProjectionMatrix() * camera->getWorldToCameraMatrix();
    }

    HookManager::HookManager() = default;

    HookManager &HookManager::GetInstance() {
        static HookManager instance;
        return instance;
    }

    int initEgl() {
        egl_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (egl_Display == EGL_NO_DISPLAY) {
            LOGE("eglGetDisplay error=%u", glGetError());
            return -1;
        }

        auto *version = new EGLint[2];
        if (!eglInitialize(egl_Display, &version[0], &version[1])) {
            LOGE("eglInitialize error=%u", glGetError());
            return -1;
        }

        const EGLint attribs[] = {EGL_BUFFER_SIZE, 32, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
                                  EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 8,
                                  EGL_STENCIL_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                                  EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};

        EGLint num_config;
        if (!eglGetConfigs(egl_Display, nullptr, 1, &num_config)) {
            LOGE("eglGetConfigs  error =%u", glGetError());
            return -1;
        }

        if (!eglChooseConfig(egl_Display, attribs, &egl_Config, 1, &num_config)) {
            LOGE("eglChooseConfig  error=%u", glGetError());
            return -1;
        }

        int attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        egl_Context = eglCreateContext(egl_Display, egl_Config, EGL_NO_CONTEXT, attrib_list);
        if (egl_Context == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext  error = %u", glGetError());
            return -1;
        }

        egl_Surface = eglCreateWindowSurface(egl_Display, egl_Config, nativeWindow, nullptr);
        if (egl_Surface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface  error = %u", glGetError());
            return -1;
        }

        if (!eglMakeCurrent(egl_Display, egl_Surface, egl_Surface, egl_Context)) {
            LOGE("eglMakeCurrent  error = %u", glGetError());
            return -1;
        }
        return 1;
    }

    extern "C"
    JNIEXPORT void JNICALL
    Java_com_android_support_JNIBridge_surfaceChanged(JNIEnv *env, jclass clazz, jint width, jint height) {
        LOGI("onSurfaceChange: [%d] x [%d]", width, height);
        Screen::glWidth = width;
        Screen::glHeight = height;
        isSurfaceChange = true;
    }

    extern "C"
    JNIEXPORT void JNICALL
    Java_com_android_support_JNIBridge_surfaceDestroyed(JNIEnv *env, jclass clazz) {
        LOGW("surfaceDestroyed!");
        isSurfaceDestroyed = true;
    }

    void InitGUI() {
        if (isFirstInit)
            return;

        LOGI("Initializing GUI...");

        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        io.IniFilename = nullptr;
        io.IniSavingRate = 1.0f;
        io.ConfigWindowsMoveFromTitleBarOnly = false;
        io.ConfigWindowsResizeFromEdges = false;

        ImGui::StyleColorsDark();

        ImGui_ImplAndroid_Init();
        ImGui_ImplOpenGL3_Init();

        ImFontConfig out_cfg;
        out_cfg.SizePixels = Screen::applyDimension(Screen::TypeValue::COMPLEX_UNIT_SP, 15.f);
        io.Fonts->AddFontDefault(&out_cfg);

        ImGui::GetStyle().ScaleAllSizes(Screen::metrics.density);
        LOGI("GUI initialized");

        isFirstInit = true;
    }

    void loopActor() {
        try {
            auto localActor = ActorHelper::GetLocalActor();
            if (!localActor)
                return;

            for (int i = 0; i < actors.size(); ++i) {
                auto actor = actors.at(i);
                if (!actor)
                    continue;

                if (localActor->IsSelfCamp(actor))
                    continue;

                auto gObj = actor->GetGameObject();
                if (!gObj)
                    continue;

                if (!gObj->IsActive())
                    continue;

                auto tr = gObj->GetTransform();
                if (!tr)
                    continue;

                Vector2 posSc;
                auto pos = tr->GetPosition();

                if (!Camera::WorldToScreen(pos, &posSc))
                    continue;

                ImGui::GetBackgroundDrawList()->AddLine({0, 0}, {posSc.X, posSc.Y}, IM_COL32_WHITE);
            }
        } catch (...) {}
    }

    void DrawGUI() {
        ImGuiIO &io = ImGui::GetIO();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame(Screen::glWidth, Screen::glHeight);

        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin(OBFUSCATE("BryanGIG")))
            return;


        ImGui::Text("Surface (%.1f FPS)", io.Framerate);
    }

    void ClearGUI() {
        ImGuiIO &io = ImGui::GetIO();
        ImGui::End();
        ImGui::Render();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void onDestroyGUI() {

        if (!isFirstInit)
            return;
        LOGI("onDestroyGUI");

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();

        if (egl_Display != EGL_NO_DISPLAY) {
            eglMakeCurrent(egl_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (egl_Context != EGL_NO_CONTEXT)
                eglDestroyContext(egl_Display, egl_Context);

            if (egl_Surface != EGL_NO_SURFACE)
                eglDestroySurface(egl_Display, egl_Surface);

            eglTerminate(egl_Display);
        }

        egl_Display = EGL_NO_DISPLAY;
        egl_Context = EGL_NO_CONTEXT;
        egl_Surface = EGL_NO_SURFACE;

        ANativeWindow_release(nativeWindow);
        isFirstInit = false;
    }

    extern "C"
    JNIEXPORT void JNICALL
    Java_com_android_support_JNIBridge_surfaceCreate(JNIEnv *env, jclass clazz, jobject surface,
                                                     jint width, jint height) {
        LOGI("surfaceCreated");

        auto handle = dlopen(OBFUSCATE("libandroid.so"), RTLD_LAZY);
        if (!handle) {
            LOGE("landroid null!");
            return;
        }

        auto fromSurface = dlsym(handle, OBFUSCATE("ANativeWindow_fromSurface"));
        if (!fromSurface) {
            LOGE("fromSurface null!");
            return;
        }

        ANativeWindow *(*ANativeWindow_fromSurface)(JNIEnv *env, jobject surface);

        ANativeWindow_fromSurface = decltype(ANativeWindow_fromSurface)(fromSurface);

        nativeWindow = ANativeWindow_fromSurface(env, surface);

        if (!nativeWindow)
            return;

        Screen::glWidth = width;
        Screen::glHeight = height;

        std::thread([&] {
            if (initEgl() == -1) {
                LOGE("Failed init egl!");
                return;
            }
            InitGUI();

            bool threadAttach = false;

            while (true) {

                if (Il2Cpp::IsReady && !threadAttach) {
                    LOGI("Attaching thread il2cpp");
                    Il2Cpp::AttachThread();
                    threadAttach = true;
                }

                if (Screen::glWidth <= 0 || Screen::glHeight <= 0 ||
                    Screen::metrics.densityDpi <= 0)
                    continue;

                if (isSurfaceChange) {
                    glViewport(0, 0, Screen::glWidth, Screen::glHeight);
                    isSurfaceChange = false;
                }

                DrawGUI();
                if (Il2Cpp::IsReady && threadAttach) {
                    loopActor();
                }
                ClearGUI();
                eglSwapBuffers(egl_Display, egl_Surface);

                if (isPause && isSurfaceDestroyed) {
                    onDestroyGUI();
                    break;
                }
            }
        }).detach();
    }

    void HookManager::StartThread() {
        std::thread([&] {
            do {
                sleep(1);
            } while (!isLibraryLoaded(targetLibName));
            sleep(15);
            if (Il2Cpp::Attach(targetLibName) == 0) {
                Il2Cpp::InitClass();
                HOOK(ActorCreators_API.CreateHeroComponent, CreateHeroComponent);
                HOOK(ActorCreators_API.DestoryHeroComponent, DestroyHeroComponent);
                HOOK(GamePlay_API.LateUpdate, GamePlay_LateUpdate);
                LOGI("Attach Success!");
            } else {
                LOGI("Attach Failed");
            }
        }).detach();
    }


    extern "C"
    JNIEXPORT void JNICALL
    Java_com_android_support_JNIBridge_onCreate(JNIEnv *env, jclass , jobject ctx) {
        LOGI("onCreate");
        auto cContext = env->GetObjectClass(ctx);
        auto mResources = env->GetMethodID(cContext, "getResources","()Landroid/content/res/Resources;");
        auto resources = env->CallObjectMethod(ctx, mResources);
        auto cResources = env->GetObjectClass(resources);
        auto mDisplayMatrics = env->GetMethodID(cResources, "getDisplayMetrics","()Landroid/util/DisplayMetrics;");
        auto displayMatrics = env->CallObjectMethod(resources, mDisplayMatrics);
        auto cDisplayMatrics = env->GetObjectClass(displayMatrics);
        auto mDensity = env->GetFieldID(cDisplayMatrics, "density", "F");
        auto mDensityDpi = env->GetFieldID(cDisplayMatrics, "densityDpi", "I");
        auto mScaledDensity = env->GetFieldID(cDisplayMatrics, "scaledDensity", "F");
        auto mXdpi = env->GetFieldID(cDisplayMatrics, "xdpi", "F");
        auto mYdpi = env->GetFieldID(cDisplayMatrics, "ydpi", "F");

        auto density = env->GetFloatField(displayMatrics, mDensity);
        auto densityDpi = env->GetIntField(displayMatrics, mDensityDpi);
        auto scaledDensity = env->GetFloatField(displayMatrics, mScaledDensity);
        auto xdpi = env->GetFloatField(displayMatrics, mXdpi);
        auto ydpi = env->GetFloatField(displayMatrics, mYdpi);

        Screen::metrics = {
                .density = density,
                .densityDpi = densityDpi,
                .scaledDensity = scaledDensity,
                .xdpi = xdpi,
                .ydpi = ydpi
        };
        LOGI("finish onCreate");
    }

    extern "C"
    JNIEXPORT jboolean JNICALL
    Java_com_android_support_JNIBridge_injectEvent(JNIEnv *env, jclass , jobject event) {
        if (isFirstInit) {
            ImGuiIO &io = ImGui::GetIO();
            ImGui_ImplAndroid_HandleInputEvent(env, event, {0,0});
            return io.WantCaptureMouse;
        }
        return false;
    }
}