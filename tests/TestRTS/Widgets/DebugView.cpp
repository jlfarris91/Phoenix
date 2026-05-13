#include "DebugView.h"

#include <imgui.h>

#include "../App/App.h"
#include "../imgui/ImGuiPropertyGrid.h"

#include "PhoenixSim/Session.h"

using namespace Phoenix;

void DebugView::Render(SessionConstRef session, WorldConstRef world)
{
    App& app = App::Get();
    auto io = ImGui::GetIO();

    auto sessionDriver = app.GetSessionDriver();
    if (!sessionDriver)
    {
        return;
    }

    // if (ImGui::BeginTable("FPS", 2, ImGuiTableFlags_SizingFixedFit))
    // {
    //     ImGui::TableNextColumn();
    //     ImGui::Text("Sim FPS:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%.3f ms/frame (%.1f FPS)", session.GetFPSCalc().GetFPS(), session.GetFPSCalc().GetFramerate());
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("SDL FPS:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%.3f ms/frame (%.1f FPS)", GRendererFPS.GetFPS(), GRendererFPS.GetFramerate());
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("ImGui FPS:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("Sim Time:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%llu (%f)", session.GetSimTime(), session.GetSimTime() / (float)Time::D);
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("World Copy:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%.3f ms/frame", GWorldView.GetUpdateRate());
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("Acc Dirty Pages:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%u pages", GWorldView.GetAccumulatedDirtyPageCount());
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("Mouse Pos:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%0.2f %0.2f", mousePos.x, mousePos.y);
    //
    //     ImGui::TableNextColumn();
    //     ImGui::Text("World Pos:");
    //     ImGui::TableNextColumn();
    //     ImGui::Text("%0.2f %0.2f", (float)worldMousePos.X, (float)worldMousePos.Y);
    //         
    //     ImGui::EndTable();
    // }
    //
    // // Session play controls
    // {
    //     static constexpr double kMinSpeed = 0.1f;
    //     static constexpr double kMaxSpeed = 16.0f;
    //     ImGui::DragScalar("Sim Speed", ImGuiDataType_Double, &GSimSpeed, 0.25, &kMinSpeed, &kMaxSpeed);
    //     ImGui::SameLine();
    //
    //     if (GTickSession)
    //     {
    //         if (ImGui::SmallButton("Pause"))
    //         {
    //             GTickSession = false;
    //         }
    //     }
    //     else
    //     {
    //         if (ImGui::SmallButton("Play"))
    //         {
    //             GTickSession = true;
    //         }
    //     }
    //
    //     ImGui::SameLine();
    //
    //     ImGui::BeginDisabled(GTickSession);
    //     if (ImGui::ArrowButton("Step", ImGuiDir_Right))
    //     {
    //         GWantsSessionStep = 1;
    //     }
    //     ImGui::EndDisabled();
    // }

    // bool copyWorld = GWorldView.IsEnabled();
    // if (ImGui::Checkbox("Copy World", &copyWorld))
    // {
    //     GWorldView.SetEnabled(copyWorld);
    // }
    //
    // if (ImGui::Checkbox("VSync", &GVsync))
    // {
    //     SDL_SetRenderVSync(GRenderer, GVsync ? 1 : 0);
    // }
    //
    // ImGui::Checkbox("Draw Grid", &GDrawGrid);
    //
    // ImGui::SliderFloat2("Scale", &GViewport->Scale.x, 0.01f, 2.0f);

    if (ImGui::CollapsingHeader("Features"))
    {
        for (const auto& feature : session.GetFeatureSet()->GetFeatures())
        {
            const TypeDescriptor& typeDescriptor = feature->GetTypeDescriptor();
            const FeatureDefinition& featureDefinition = feature->GetFeatureDefinition();

            if (ImGui::CollapsingHeader(typeDescriptor.GetDisplayName().c_str()))
            {
                if (ImGui::TreeNode("Properties:"))
                {
                    DrawPropertyGrid(feature.get(), typeDescriptor);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Session Blocks:"))
                {
                    for (const BufferBlockDefinition& blockDef : featureDefinition.SessionBlocks.Definitions)
                    {
                        const uint8* block = session.GetBlock(blockDef.TypeName);
                        if (!block || !blockDef.Type)
                        {
                            continue;
                        }

                        if (ImGui::TreeNode(blockDef.Type->GetDisplayName().c_str()))
                        {
                            DrawPropertyGrid(block, *blockDef.Type);
                            ImGui::TreePop();
                        }
                    }
                        
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("World Blocks:"))
                {
                    for (const BufferBlockDefinition& blockDef : featureDefinition.WorldBlocks.Definitions)
                    {
                        const uint8* block = world.GetBlock(blockDef.TypeName);
                        if (!block || !blockDef.Type)
                        {
                            continue;
                        }

                        if (ImGui::TreeNode(blockDef.Type->GetDisplayName().c_str()))
                        {
                            DrawPropertyGrid(block, *blockDef.Type);
                            ImGui::TreePop();
                        }
                    }
                        
                    ImGui::TreePop();
                }
            }
        }
    }
}
