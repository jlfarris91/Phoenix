#include "AppView.h"

#include <imgui.h>

#include "../App/App.h"
#include "../App/SessionDriver.h"
#include "../App/SessionInstance.h"

#include "SessionView.h"

#include "PhoenixLua/FeatureLua.h"

#include "PhoenixPhysics/FeaturePhysics.h"

#include "PhoenixRTS/Abilities/FeatureAbilities.h"
#include "PhoenixRTS/Abilities/Attack/AttackAbilityHandler.h"
#include "PhoenixRTS/Abilities/Move/MoveAbilityHandler.h"
#include "PhoenixRTS/Effects/EffectDamageHandler.h"
#include "PhoenixRTS/Effects/EffectLaunchProjectileHandler.h"
#include "PhoenixRTS/Effects/FeatureEffects.h"
#include "PhoenixRTS/Effects/ResponseDamageHandler.h"
#include "PhoenixRTS/Effects/Responses.h"
#include "PhoenixRTS/Orders/FeatureOrders.h"
#include "PhoenixRTS/Projectiles/FeatureProjectile.h"
#include "PhoenixRTS/Selection/FeatureSelection.h"
#include "PhoenixRTS/Units/FeatureUnit.h"

#include "PhoenixScript/FeatureScript.h"

#include "PhoenixSim/Blackboard/FeatureBlackboard.h"
#include "PhoenixSim/Debug/FeatureDebug.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"
#include "PhoenixSim/Navigation/FeatureNavigation.h"
#include "PhoenixSim/Services/ServiceContainerBuilder.h"
#include "PhoenixSim/Strings/FeatureString.h"
#include "PhoenixSim/Tasks/FeatureTask.h"

#include "PhoenixSteering/FeatureSteering.h"

using namespace Phoenix;

void AppView::Initialize()
{
    Widget::Initialize();

    if (SessionDriver* sessionDriver = App::Get().GetSessionDriver())
    {
        sessionDriver->OnSessionCreated.AddSP(shared_from_this(), &AppView::OnSessionCreated);
    }
}

void AppView::Render()
{
    Widget::Render();

    RenderMenuBar();
    RenderDockSpace();
}

void AppView::OnAppEvent(const void* eventData)
{
    
}

void AppView::RenderMenuBar()
{
    ImGui::BeginMainMenuBar();
    
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New Session"))
        {
            HandleNewSession();
        }

        if (ImGui::MenuItem("Exit"))
        {
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void AppView::RenderDockSpace()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags hostFlags =
        ImGuiWindowFlags_NoDocking             |
        ImGuiWindowFlags_NoTitleBar            |
        ImGuiWindowFlags_NoCollapse            |
        ImGuiWindowFlags_NoResize              |
        ImGuiWindowFlags_NoMove                |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus            ;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0.0f, 0.0f));
    ImGui::Begin("##RootDockSpace", nullptr, hostFlags);
    ImGui::PopStyleVar(3);

    RenderSessionWindows();

    ImGui::End();
}

void AppView::RenderSessionWindows()
{
    App& app = App::Get();

    auto sessionDriver = app.GetSessionDriver();
    if (!sessionDriver)
    {
        return;
    }

    auto sessions = sessionDriver->GetSessions();

    for (auto&& session : sessions)
    {
        RenderSessionWindow(session);
    }
}

void AppView::RenderSessionWindow(SessionInstance* value)
{
    ImGui::PushID(value->GetId());
    ImGui::Begin("##session");

    auto sessionViewIter = SessionViews.find(value->GetId());
    if (sessionViewIter != SessionViews.end())
    {
        sessionViewIter->second->Render();
    }

    ImGui::End();
    ImGui::PopID();
}

void AppView::HandleNewSession()
{
    std::shared_ptr<ServiceContainerBuilder> serviceContainerBuilder = std::make_shared<ServiceContainerBuilder>();

    // Register features
    // ReSharper disable CppExpressionWithoutSideEffects
    serviceContainerBuilder->RegisterService<FeatureString>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureDebug>().AsInterfaces();
    serviceContainerBuilder->RegisterService<Blackboard::FeatureBlackboard>().AsInterfaces();
    serviceContainerBuilder->RegisterService<LDS::FeatureLDS>().AsInterfaces();
    serviceContainerBuilder->RegisterService<ECS::FeatureECS>().AsInterfaces();
    serviceContainerBuilder->RegisterService<Tasks::FeatureTask>().AsInterfaces();
    serviceContainerBuilder->RegisterService<Pathfinding::FeatureNavigation>().AsInterfaces();
    serviceContainerBuilder->RegisterService<Physics::FeaturePhysics>().AsInterfaces();
    serviceContainerBuilder->RegisterService<Steering::FeatureSteering>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureLua>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureScript>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureUnit>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureAbilities>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureEffects>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureOrders>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureSelection>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureProjectiles>().AsInterfaces();

    // Register ability handlers
    serviceContainerBuilder->RegisterService<RTS::MoveAbilityHandler>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::AttackAbilityHandler>().AsInterfaces();

    // Register effect handlers
    serviceContainerBuilder->RegisterService<RTS::EffectDamageHandler>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::EffectLaunchProjectileHandler>().AsInterfaces();

    // Register response handlers
    serviceContainerBuilder->RegisterService<RTS::ResponseHandlerBase>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::ResponseDamageHandler>().AsInterfaces();

    SessionCtorArgs sessionArgs;
    sessionArgs.DataDirectory = "./Data";
    sessionArgs.ConfigName = "DefaultSession";
    sessionArgs.ServiceContainerBuilder = serviceContainerBuilder;

    App::Get().GetSessionDriver()->CreateSession(SessionCtorArgs());
}

void AppView::HandleExit()
{
    
}

void AppView::OnSessionCreated(SessionInstance* value)
{
    assert(SessionViews.find(value->GetId()) == SessionViews.end());

    auto sessionView = std::make_unique<SessionView>(value);
    auto result = SessionViews.emplace(value->GetId(), std::move(sessionView));
    assert(result.second);

    result.first->second->Initialize();
}

void AppView::OnSessionDestroyed(SessionInstance* value)
{
    SessionViews.erase(value->GetId());
}
