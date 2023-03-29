#pragma once

#ifndef _MTHREADRENDERER_H
#define _MTHREADRENDERER_H

#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <mutex>
#include <vector>

namespace cheat {
    namespace gui {
        // from imgui issue: https://github.com/ocornut/imgui/issues/5776
        class MThreadRenderer {
        private:
            std::mutex m_mutex;
            ImDrawData m_drawData;

            std::vector<ImDrawList> m_drawLists;    // the thread render to these draw lists
            std::vector<ImDrawList> m_copyList;     // calling EndFrame() will copy the m_drawLists to this
            std::vector<ImDrawList *> m_copyPointer; // pointer to each drawlist in m_copyList, needed for m_drawData

            ImGuiViewport *m_viewport;
        public:

            // numDrawList = number of draw lists needed, 2 for background and foreground
            MThreadRenderer(size_t numDrawList = 2, ImGuiViewport *viewport = nullptr);

            static MThreadRenderer& GetInstance();

            // index=0 for the background drawlist, index=1 for the foreground, basically the z-index
            ImDrawList &GetDrawList(size_t index = 0) noexcept;

            void BeginFrame();

            void EndFrame();

            void Render() noexcept;
        };
    } // cheat
} // gui

#endif //_MTHREADRENDERER_H
