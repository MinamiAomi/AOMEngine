#include "EditerManager.h"

#include <Windows.h>
#include <d3d12.h>

#ifdef ENABLE_IMGUI
#include "Externals/ImGui/imgui.h"
#include "Externals/ImGui/imgui_stdlib.h"
#include "Externals/ImGui/imgui_impl_dx12.h"
#include "Externals/ImGui/imgui_impl_win32.h"
#endif // ENABLE_IMGUI


#include "Framework/Engine.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/GameWindow.h"
#include "Graphics/RenderManager.h"

namespace Editer {

    EditerManager::EditerManager() :
        hierarchyView_(std::make_unique<HierarchyView>(*this)),
        inspectorView_(std::make_unique<InspectorView>(*this)),
        sceneView_(std::make_unique<SceneView>(*this)),
        projectView_(std::make_unique<ProjectView>(*this)),
        consoleView_(std::make_unique<ConsoleView>(*this)) {
    }

    void EditerManager::Initialize() {
        auto graphics = Engine::GetGraphics();
        auto window = Engine::GetGameWindow();
        auto renderManager = Engine::GetRenderManager();

        descriptor_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui::StyleColorsDark();
        ImGui::StyleColorsClassic();
        ImGui_ImplWin32_Init(window->GetHWND());
        ImGui_ImplDX12_Init(
            graphics->GetDevice(),
            SwapChain::kNumBuffers,
            renderManager->GetSwapChain().GetColorBuffer(0).GetRTVFormat(),
            graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
            descriptor_,
            descriptor_);
    }

    void EditerManager::Render() {
        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX12_NewFrame();
        ImGui::NewFrame();

        RenderDockingSpace();

        auto gameObjectManager = Engine::GetGameObjectManager();
        hierarchyView_->Render(*gameObjectManager);
        inspectorView_->Render(selectedObject_);
        consoleView_->Render();
    }

    void EditerManager::RenderToColorBuffer(CommandContext& commandContext) {
        projectView_->Render();
        sceneView_->Render(commandContext);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext);
    }

    void EditerManager::Finalize() {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditerManager::RenderDockingSpace() {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // ウィンドウフラグの設定
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // メインドッキングスペースの開始
        ImGui::Begin("DockSpace", nullptr, window_flags);

        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // ドッキングスペースの設定
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        // メニューバーの例
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Test")) {
                ImGui::MenuItem("Test", NULL, false, false); // 無効なメニューアイテム
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) {
                if (ImGui::MenuItem("Hierarchy", NULL, hierarchyView_->isDisplayed)) {
                    hierarchyView_->isDisplayed = true;
                }
                if (ImGui::MenuItem("Inspector", NULL, inspectorView_->isDisplayed)) {
                    inspectorView_->isDisplayed = true;
                }
                if (ImGui::MenuItem("Scene", NULL, sceneView_->isDisplayed)) {
                    sceneView_->isDisplayed = true;
                }
                if (ImGui::MenuItem("Project", NULL, projectView_->isDisplayed)) {
                    projectView_->isDisplayed = true;
                }
                if (ImGui::MenuItem("Console", NULL, consoleView_->isDisplayed)) {
                    consoleView_->isDisplayed = true;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

}