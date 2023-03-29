#include "MThreadRenderer.h"

namespace cheat {
    namespace gui {
    MThreadRenderer::MThreadRenderer(size_t numDrawList, ImGuiViewport *viewport) {
        m_viewport = static_cast<ImGuiViewport *>(viewport ? viewport
                                                           : ImGui::GetMainViewport());

        m_copyList.resize(numDrawList, ImGui::GetDrawListSharedData());

        for (size_t i = 0; i < numDrawList; i++) {
            m_drawLists.emplace_back(ImGui::GetDrawListSharedData());
            m_copyPointer.emplace_back(&m_copyList[i]);
        }

        m_drawData.Valid = true;
        m_drawData.CmdLists = m_copyPointer.data();
        m_drawData.CmdListsCount = m_copyPointer.size();
    }

    ImDrawList &MThreadRenderer::GetDrawList(size_t index) noexcept {
        return m_drawLists[index];
    }

    void MThreadRenderer::BeginFrame() {
            for (auto &drawList: m_drawLists) {
                drawList._ResetForNewFrame();
                drawList.PushTextureID(ImGui::GetIO().Fonts->TexID);
                auto clip_rect_max = ImVec2(m_viewport->Pos.x + m_viewport->Size.x,
                                            m_viewport->Pos.y + m_viewport->Size.y);
                drawList.PushClipRect(m_viewport->Pos, clip_rect_max, false);
            }
    }

    void MThreadRenderer::Render() noexcept {
        m_mutex.lock();
        ImGui_ImplOpenGL3_RenderDrawData(&m_drawData);
        m_mutex.unlock();
    }

    void MThreadRenderer::EndFrame() {
        m_mutex.lock();

        m_copyList = m_drawLists;

        m_drawData.TotalVtxCount = m_drawData.TotalIdxCount = 0;
        m_drawData.DisplayPos = m_viewport->Pos;
        m_drawData.DisplaySize = m_viewport->Size;
        m_drawData.FramebufferScale = ImGui::GetIO().DisplayFramebufferScale;

        for (auto &drawList: m_copyList) {
            drawList._PopUnusedDrawCmd();
            m_drawData.TotalVtxCount += drawList.VtxBuffer.Size;
            m_drawData.TotalIdxCount += drawList.IdxBuffer.Size;
        }

        m_mutex.unlock();
    }

    MThreadRenderer &MThreadRenderer::GetInstance() {
        static MThreadRenderer instance;
        return instance;
    }

} } // cheat::gui
