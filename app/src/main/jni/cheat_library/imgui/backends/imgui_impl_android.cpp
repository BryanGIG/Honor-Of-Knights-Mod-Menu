// dear vendor.imgui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (imgui_impl_opengl3)

// Implemented features:
//  [X] Platform: Keyboard arrays indexed using AKEYCODE_* codes, e.g. ImGui::IsKeyPressed(AKEYCODE_SPACE).
// Missing features:
//  [ ] Platform: Clipboard support.
//  [ ] Platform: Gamepad support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [ ] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'. FIXME: Check if this is even possible with Android.
// Important:
//  - FIXME: On-screen keyboard currently needs to be enabled by the application (see examples/ and issue #3446)
//  - FIXME: Unicode character inputs needs to be passed by Dear ImGui by the application (see examples/ and issue #3446)

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire vendor.imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of vendor.imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2021-03-04: Initial version.

#include "imgui.h"
#include "imgui_impl_android.h"
#include <ctime>
#include <map>
#include <queue>
#include <android/native_window.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <dlfcn.h>

// Android data
static double g_Time = 0.0;
static std::map<int32_t, std::queue<int32_t>> g_KeyEventQueues; // FIXME: Remove dependency on map and queue once we use upcoming input queue.

#define find_class(name) env->FindClass(name)
#define get_method(claz, name, param) env->GetMethodID(claz, name, param)

int32_t ImGui_ImplAndroid_HandleInputEvent(JNIEnv *env, jobject input_event, ImVec2 scale) {

    ImGuiIO &io = ImGui::GetIO();

    auto keyEvent_clazz = find_class("android/view/KeyEvent");
    auto motionEvent_clazz = find_class("android/view/MotionEvent");

    if (env->IsInstanceOf(input_event, keyEvent_clazz)) {
        auto getAction = get_method(keyEvent_clazz, "getAction", "()I");
        auto getKeyCode = get_method(keyEvent_clazz, "getKeyCode", "()I");
        auto getMetaState = get_method(keyEvent_clazz, "getMetaState", "()I");

        int32_t event_key_code = env->CallIntMethod(input_event, getKeyCode);
        int32_t event_action = env->CallIntMethod(input_event, getAction);
        int32_t event_meta_state = env->CallIntMethod(input_event, getMetaState);

        io.KeyCtrl = ((event_meta_state & AMETA_CTRL_ON) != 0);
        io.KeyShift = ((event_meta_state & AMETA_SHIFT_ON) != 0);
        io.KeyAlt = ((event_meta_state & AMETA_ALT_ON) != 0);

        switch (event_action) {
            case AKEY_EVENT_ACTION_DOWN:
            case AKEY_EVENT_ACTION_UP:
                g_KeyEventQueues[event_key_code].push(event_action);
                break;
            default:
                break;
        }

        return 1;
    } else if (env->IsInstanceOf(input_event, motionEvent_clazz)) {
        auto getAction2 = get_method(motionEvent_clazz, "getAction", "()I");
        auto getX = get_method(motionEvent_clazz, "getX", "()F");
        auto getY = get_method(motionEvent_clazz, "getY", "()F");

        int32_t event_action = env->CallIntMethod(input_event, getAction2);
        int32_t event_pointer_index = (event_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        event_action &= AMOTION_EVENT_ACTION_MASK;
        switch (event_action) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_UP: {
                // Physical mouse buttons (and probably other physical devices) also invoke the actions AMOTION_EVENT_ACTION_DOWN/_UP,
                // but we have to process them separately to identify the actual button pressed. This is done below via
                // AMOTION_EVENT_ACTION_BUTTON_PRESS/_RELEASE. Here, we only process "FINGER" input (and "UNKNOWN", as a fallback).
                auto getToolType = get_method(motionEvent_clazz, "getToolType", "(I)I");

                int32_t tool_type = env->CallIntMethod(input_event, getToolType, event_pointer_index);
                if ((tool_type == AMOTION_EVENT_TOOL_TYPE_FINGER) ||
                    (tool_type == AMOTION_EVENT_TOOL_TYPE_UNKNOWN)) {
                    float posX = env->CallFloatMethod(input_event, getX);
                    float posY = env->CallFloatMethod(input_event, getY);
                    io.MouseDown[0] = (event_action == AMOTION_EVENT_ACTION_DOWN);
                    io.MousePos = ImVec2(scale.x > 0 ? posX / scale.x : posX, scale.y > 0 ? posY / scale.y : posY);
                }
            }
                break;
            case AMOTION_EVENT_ACTION_BUTTON_PRESS:
            case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
                auto getButtonState = get_method(motionEvent_clazz, "getButtonState", "()I");
                int32_t button_state = env->CallIntMethod(input_event, getButtonState);
                io.MouseDown[0] = ((button_state & AMOTION_EVENT_BUTTON_PRIMARY) != 0);
                io.MouseDown[1] = ((button_state & AMOTION_EVENT_BUTTON_SECONDARY) != 0);
                io.MouseDown[2] = ((button_state & AMOTION_EVENT_BUTTON_TERTIARY) != 0);
            }
                break;
            case AMOTION_EVENT_ACTION_HOVER_MOVE: // Hovering: Tool moves while NOT pressed (such as a physical mouse)
            case AMOTION_EVENT_ACTION_MOVE: {       // Touch pointer moves while DOWN
                float posX = env->CallFloatMethod(input_event, getX);
                float posY = env->CallFloatMethod(input_event, getY);
                io.MousePos = ImVec2(scale.x > 0 ? posX / scale.x : posX,scale.y > 0 ? posY / scale.y : posY);
            }
                break;
            case AMOTION_EVENT_ACTION_SCROLL: {
                auto getAxisValue = get_method(motionEvent_clazz, "getAxisValue", "(I)F");
                io.MouseWheel = env->CallFloatMethod(input_event, getAxisValue, (int) AMOTION_EVENT_AXIS_VSCROLL);
                io.MouseWheelH = env->CallFloatMethod(input_event, getAxisValue, (int) AMOTION_EVENT_AXIS_HSCROLL);
            }
                break;
            default:
                break;

        }

        return 1;
    }
    return 0;
}

#define INIT_ANDROID_API(name, str) fake_InputEvent.name = decltype(fake_InputEvent.name)(dlsym(android, str))

struct FNativeInput {
    int32_t (*AInputEvent_getType)(const AInputEvent *event);
    int32_t (*AKeyEvent_getKeyCode)(const AInputEvent *key_event);
    int32_t (*AKeyEvent_getAction)(const AInputEvent *key_event);
    int32_t (*AKeyEvent_getMetaState)(const AInputEvent* key_event);
    int32_t (*AMotionEvent_getAction)(const AInputEvent *pEvent);
    int32_t (*AMotionEvent_getToolType)(const AInputEvent* motion_event, size_t pointer_index);
    float (*AMotionEvent_getX)(const AInputEvent* motion_event, size_t pointer_index);
    float (*AMotionEvent_getY)(const AInputEvent* motion_event, size_t pointer_index);
    int32_t (*AMotionEvent_getButtonState)(const AInputEvent* motion_event);
    float (*AMotionEvent_getAxisValue)(const AInputEvent* motion_event, int32_t axis, size_t pointer_index);
} fake_InputEvent;

void initFakeInput() {
    auto android = dlopen("libandroid.so", RTLD_LAZY);
    if (!android)
        return;

    INIT_ANDROID_API(AInputEvent_getType, "AInputEvent_getType");
    INIT_ANDROID_API(AKeyEvent_getKeyCode, "AKeyEvent_getKeyCode");
    INIT_ANDROID_API(AKeyEvent_getAction, "AKeyEvent_getAction");
    INIT_ANDROID_API(AKeyEvent_getMetaState, "AKeyEvent_getMetaState");
    INIT_ANDROID_API(AMotionEvent_getAction, "AMotionEvent_getAction");
    INIT_ANDROID_API(AMotionEvent_getToolType, "AMotionEvent_getToolType");
    INIT_ANDROID_API(AMotionEvent_getX, "AMotionEvent_getX");
    INIT_ANDROID_API(AMotionEvent_getY, "AMotionEvent_getY");
    INIT_ANDROID_API(AMotionEvent_getButtonState, "AMotionEvent_getButtonState");
}

int32_t ImGui_ImplAndroid_HandleAInputEvent(AInputEvent* input_event, ImVec2 scale) {
    initFakeInput();

    ImGuiIO &io = ImGui::GetIO();
    int32_t event_type = fake_InputEvent.AInputEvent_getType(input_event);
    switch (event_type) {
        case AINPUT_EVENT_TYPE_KEY: {
            int32_t event_key_code = fake_InputEvent.AKeyEvent_getKeyCode(input_event);
            int32_t event_action = fake_InputEvent.AKeyEvent_getAction(input_event);
            int32_t event_meta_state = fake_InputEvent.AKeyEvent_getMetaState(input_event);

            io.KeyCtrl = ((event_meta_state & AMETA_CTRL_ON) != 0);
            io.KeyShift = ((event_meta_state & AMETA_SHIFT_ON) != 0);
            io.KeyAlt = ((event_meta_state & AMETA_ALT_ON) != 0);

            switch (event_action) {
                // FIXME: AKEY_EVENT_ACTION_DOWN and AKEY_EVENT_ACTION_UP occur at once as soon as a touch pointer
                // goes up from a key. We use a simple key event queue/ and process one event per key per frame in
                // ImGui_ImplAndroid_NewFrame()...or consider using IO queue, if suitable: https://github.com/ocornut/imgui/issues/2787
                case AKEY_EVENT_ACTION_DOWN:
                case AKEY_EVENT_ACTION_UP:
                    g_KeyEventQueues[event_key_code].push(event_action);
                    break;
                default:
                    break;
            }
            break;
        }
        case AINPUT_EVENT_TYPE_MOTION: {
            int32_t event_action = fake_InputEvent.AMotionEvent_getAction(input_event);
            int32_t event_pointer_index = (event_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
            event_action &= AMOTION_EVENT_ACTION_MASK;
            switch (event_action) {
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_UP:
                    // Physical mouse buttons (and probably other physical devices) also invoke the actions AMOTION_EVENT_ACTION_DOWN/_UP,
                    // but we have to process them separately to identify the actual button pressed. This is done below via
                    // AMOTION_EVENT_ACTION_BUTTON_PRESS/_RELEASE. Here, we only process "FINGER" input (and "UNKNOWN", as a fallback).
                    if ((fake_InputEvent.AMotionEvent_getToolType(input_event, event_pointer_index) == AMOTION_EVENT_TOOL_TYPE_FINGER) || (fake_InputEvent.AMotionEvent_getToolType(input_event, event_pointer_index) == AMOTION_EVENT_TOOL_TYPE_UNKNOWN)) {
                        io.MouseDown[0] = (event_action == AMOTION_EVENT_ACTION_DOWN);
                        auto xPos = fake_InputEvent.AMotionEvent_getX(input_event, event_pointer_index);
                        auto yPos = fake_InputEvent.AMotionEvent_getY(input_event, event_pointer_index);
                        io.MousePos = ImVec2(scale.x > 0 ? xPos / scale.x : xPos,scale.y > 0 ? yPos / scale.y : yPos);
                    }
                    break;
                case AMOTION_EVENT_ACTION_BUTTON_PRESS:
                case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
                    int32_t button_state = fake_InputEvent.AMotionEvent_getButtonState(input_event);
                    io.MouseDown[0] = ((button_state & AMOTION_EVENT_BUTTON_PRIMARY) != 0);
                    io.MouseDown[1] = ((button_state & AMOTION_EVENT_BUTTON_SECONDARY) != 0);
                    io.MouseDown[2] = ((button_state & AMOTION_EVENT_BUTTON_TERTIARY) != 0);
                }
                    break;
                case AMOTION_EVENT_ACTION_HOVER_MOVE: // Hovering: Tool moves while NOT pressed (such as a physical mouse)
                case AMOTION_EVENT_ACTION_MOVE: {       // Touch pointer moves while DOWN
                    auto xPos = fake_InputEvent.AMotionEvent_getX(input_event, event_pointer_index);
                    auto yPos = fake_InputEvent.AMotionEvent_getY(input_event, event_pointer_index);
                    io.MousePos = ImVec2(scale.x > 0 ? xPos / scale.x : xPos,scale.y > 0 ? yPos / scale.y : yPos);
                }
                    break;
                case AMOTION_EVENT_ACTION_SCROLL:
                    io.MouseWheel = fake_InputEvent.AMotionEvent_getAxisValue(input_event,AMOTION_EVENT_AXIS_VSCROLL,event_pointer_index);
                    io.MouseWheelH = fake_InputEvent.AMotionEvent_getAxisValue(input_event,AMOTION_EVENT_AXIS_HSCROLL,event_pointer_index);
                    break;
                default:
                    break;
            }
        }
            return 1;
        default:
            break;
    }

    return 0;
}

bool ImGui_ImplAndroid_Init() {
    g_Time = 0.0;

    // Setup backend capabilities flags
    ImGuiIO &io = ImGui::GetIO();
    io.BackendPlatformName = "UBackend";

    // Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = AKEYCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = AKEYCODE_DPAD_LEFT;   // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_RightArrow] = AKEYCODE_DPAD_RIGHT; // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_UpArrow] = AKEYCODE_DPAD_UP;       // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_DownArrow] = AKEYCODE_DPAD_DOWN;   // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_PageUp] = AKEYCODE_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = AKEYCODE_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = AKEYCODE_MOVE_HOME;
    io.KeyMap[ImGuiKey_End] = AKEYCODE_MOVE_END;
    io.KeyMap[ImGuiKey_Insert] = AKEYCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = AKEYCODE_FORWARD_DEL;
    io.KeyMap[ImGuiKey_Backspace] = AKEYCODE_DEL;
    io.KeyMap[ImGuiKey_Space] = AKEYCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = AKEYCODE_ENTER;
    io.KeyMap[ImGuiKey_Escape] = AKEYCODE_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = AKEYCODE_NUMPAD_ENTER;
    io.KeyMap[ImGuiKey_A] = AKEYCODE_A;
    io.KeyMap[ImGuiKey_C] = AKEYCODE_C;
    io.KeyMap[ImGuiKey_V] = AKEYCODE_V;
    io.KeyMap[ImGuiKey_X] = AKEYCODE_X;
    io.KeyMap[ImGuiKey_Y] = AKEYCODE_Y;
    io.KeyMap[ImGuiKey_Z] = AKEYCODE_Z;

    return true;
}

void ImGui_ImplAndroid_Shutdown() {
}

void ImGui_ImplAndroid_NewFrame(int width, int height) {
    ImGuiIO &io = ImGui::GetIO();

    // Process queued key events
    // FIXME: This is a workaround for multiple key event actions occurring at once (see above) and can be removed once we use upcoming input queue.
    for (auto &key_queue : g_KeyEventQueues) {
        if (key_queue.second.empty())
            continue;
        io.KeysDown[key_queue.first] = (key_queue.second.front() == AKEY_EVENT_ACTION_DOWN);
        key_queue.second.pop();
    }

    // Setup display size (every frame to accommodate for window resizing)
    int32_t window_width = width;
    int32_t window_height = height;
    int display_width = window_width;
    int display_height = window_height;

    io.DisplaySize = ImVec2((float) window_width, (float) window_height);
    if (window_width > 0 && window_height > 0)
        io.DisplayFramebufferScale = ImVec2((float) display_width / window_width,
                                            (float) display_height / window_height);

    // Setup time step
    struct timespec current_timespec{};
    clock_gettime(CLOCK_MONOTONIC, &current_timespec);
    double current_time = (double) (current_timespec.tv_sec) + (current_timespec.tv_nsec / 1000000000.0);
    io.DeltaTime = g_Time > 0.0 ? (float) (current_time - g_Time) : (float) (1.0f / 60.0f);
    g_Time = current_time;
}
