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

    static bool espLine;
    static bool espBox;
    static bool espHealth;
    static bool espExp;
    static bool espArmor;
    static bool espName;
    static float healthBarHeight = 10.f;

#define VEC2_TO_IMVEC2(vec2) ImVec2(vec2.X, vec2.Y)
#define VEC2_TO_VEC3(vec2) Vector3(vec2.X, vec2.Y, 0)
    void Box2D(Rect box, ImColor color) {
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(box.x, box.y), ImVec2(box.x + box.width, box.y + box.height),
                                                color,
                                                0.f, 15, 1.5f);
    }

    ImColor GetHealthColor(float health, float maxHp = 100.f) {
        float r = std::min(510 * (maxHp - health) / maxHp, 255.f);
        float g = std::min(510 * health / maxHp, 255.f);
        return ImVec4(r / 255.f, g / 255.f, 0.f, 1.0f);
    }

#define toPercentage(num, maxNum) (float) num / (float) maxNum * 100.f

    void DrawHealthBar(float currentHealth, float maxHealth, ImVec2 enemyPos) {
        auto barSize = ImVec2(100, 10);
        auto fillWidth = barSize.x * (currentHealth / maxHealth);
        auto screenPos = ImVec2(enemyPos.x - barSize.x / 2, enemyPos.y + barSize.y / 2);
        auto drawList = ImGui::GetBackgroundDrawList();

        drawList->AddRectFilled(screenPos, ImVec2(screenPos.x + barSize.x, screenPos.y + barSize.y), IM_COL32(0,0,0, 180.f));
        drawList->AddRectFilled(screenPos, ImVec2(screenPos.x + fillWidth, screenPos.y + barSize.y), GetHealthColor(currentHealth, maxHealth));
    }

    void loopActor() {
        try {
            auto bgDrawList = ImGui::GetBackgroundDrawList();
            auto localActor = ActorHelper::GetLocalActor();
            if (!localActor)
                return;

            auto localObj = localActor->GetGameObject();
            if (!localObj && !localObj->IsActive())
                return;

            auto localTr = localObj->GetTransform();
            if (!localTr)
                return;

            Vector2 localPosSc;
            auto localPos = localTr->GetPosition();

            Camera::WorldToScreen(localPos, &localPosSc);

            for (auto actor : actors) {
                if (!actor)
                    continue;

                if (localActor->IsSelfCamp(actor))
                    continue;

                auto valConfig = actor->GetValueComponent();
                if (!valConfig)
                    continue;

                auto hpPercent = toPercentage(valConfig->GetHp(), valConfig->GetMaxHp());
                if (hpPercent <= 0.f)
                    continue;

                Vector2 posSc;
                auto pos = actor->GetPosition();

                if (!Camera::WorldToScreen(pos, &posSc))
                    continue;


                if (espLine) {
                    bgDrawList->AddLine(VEC2_TO_IMVEC2(localPosSc), VEC2_TO_IMVEC2(posSc), IM_COL32_BLACK, 2.f);
                }

                if (espBox) {
                    auto renderer = actor->GetRenderer();
                    if (renderer) {
                        auto bounds = renderer->GetBounds();

                        auto screenCorners = new Vector2[8];
                        auto corners = new Vector3[8];
                        corners[0] = bounds.center + Vector3(bounds.extents.X, bounds.extents.Y, bounds.extents.Z);
                        corners[1] = bounds.center + Vector3(bounds.extents.X, bounds.extents.Y, -bounds.extents.Z);
                        corners[2] = bounds.center + Vector3(bounds.extents.X, -bounds.extents.Y, bounds.extents.Z);
                        corners[3] = bounds.center + Vector3(bounds.extents.X, -bounds.extents.Y, -bounds.extents.Z);
                        corners[4] = bounds.center + Vector3(-bounds.extents.X, bounds.extents.Y, bounds.extents.Z);
                        corners[5] = bounds.center + Vector3(-bounds.extents.X, bounds.extents.Y, -bounds.extents.Z);
                        corners[6] = bounds.center + Vector3(-bounds.extents.X, -bounds.extents.Y, bounds.extents.Z);
                        corners[7] = bounds.center + Vector3(-bounds.extents.X, -bounds.extents.Y, -bounds.extents.Z);

                        for (int i = 0; i < 8; i++) {
                            Camera::WorldToScreen(corners[i], &screenCorners[i]);
                        }

                        auto min = screenCorners[0];
                        auto max = screenCorners[0];
                        for (int i = 1; i < 8; i++) {
                            min = Vector2::Min(min, screenCorners[i]);
                            max = Vector2::Max(max, screenCorners[i]);
                        }

                        bgDrawList->AddRect(VEC2_TO_IMVEC2(min), VEC2_TO_IMVEC2(max), IM_COL32_WHITE, 2, 0, 1.5f);
                    }
                }

                if (espHealth) {
                    DrawHealthBar(hpPercent, 100, VEC2_TO_IMVEC2(posSc));
                }

                if (espName || espArmor || espExp) {
                    std::string textDraw;
                    if (espName) {
                        textDraw += "Name: " + actor->GetName();
                        textDraw += "\n";
                    }

                    if (espArmor) {
                        textDraw += "Armor: " + std::to_string(valConfig->GetArmor()) + " | " +std::to_string(valConfig->GetMaxArmor());
                        textDraw += "\n";
                    }

                    if (espExp) {
                        textDraw += "Exp: " + std::to_string(valConfig->GetExp()) + " | " +std::to_string(valConfig->GetMaxExp());
                        textDraw += "\n";
                    }

                    ImVec2 textSize = ImGui::CalcTextSize(textDraw.c_str());
                    ImVec2 center = {posSc.X - 10, posSc.Y + 10};
                    ImVec2 textPos = ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);
                    bgDrawList->AddText(textPos, IM_COL32_WHITE, textDraw.c_str());
                }
            }
        } catch (...) {}
    }

    void DrawGUI() {
        ImGuiIO &io = ImGui::GetIO();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame(Screen::glWidth, Screen::glHeight);

        ImGui::NewFrame();

        ImGui::SetNextWindowSize({0, 0}, ImGuiCond_FirstUseEver);

        if (!ImGui::Begin(OBFUSCATE("HoK Mod Menu (Testing)")))
            return;

        ImGui::Checkbox("ESP Line", &espLine);
        ImGui::Checkbox("ESP Box", &espBox);
        ImGui::Checkbox("ESP Health", &espHealth);
        ImGui::Checkbox("ESP Exp", &espExp);
        ImGui::Checkbox("ESP Armor", &espArmor);
        ImGui::Checkbox("ESP Name", &espName);
        ImGui::Text("Application: (%.1f FPS)", io.Framerate);
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

    extern "C"
    JNIEXPORT void JNICALL
    Java_com_android_support_JNIBridge_onPause(JNIEnv *env, jclass clazz) {
        isPause = true;
        LOGW("Pause");
    }

    extern "C"
    JNIEXPORT void JNICALL
    Java_com_android_support_JNIBridge_onResume(JNIEnv *env, jclass clazz) {
        isPause = false;
        LOGW("Resume");
    }
}