#include "SessionModule.h"

#include "SceneManager.h"
#include "Application/Application.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"
#include "Phoenix.Runtime/Sessions/SessionDriverService.h"

#include "Phoenix.Sim.Blackboard/FeatureBlackboard.h"
#include "Phoenix.Sim.Debug/FeatureDebug.h"
#include "Phoenix.Sim.ECS/FeatureECS.h"
#include "Phoenix.Sim.LDS/FeatureLDS.h"
#include "Phoenix.Sim.Lua/FeatureLua.h"
#include "Phoenix.Sim.Nav/FeatureNavigation.h"
#include "Phoenix.Sim.Physics/FeaturePhysics.h"
#include "Phoenix.Sim.RTS/Abilities/FeatureAbilities.h"
#include "Phoenix.Sim.RTS/Abilities/Attack/AttackAbilityHandler.h"
#include "Phoenix.Sim.RTS/Abilities/Move/MoveAbilityHandler.h"
#include "Phoenix.Sim.RTS/Effects/EffectDamageHandler.h"
#include "Phoenix.Sim.RTS/Effects/EffectLaunchProjectileHandler.h"
#include "Phoenix.Sim.RTS/Effects/FeatureEffects.h"
#include "Phoenix.Sim.RTS/Effects/ResponseDamageHandler.h"
#include "Phoenix.Sim.RTS/Effects/Responses.h"
#include "Phoenix.Sim.RTS/Orders/FeatureOrders.h"
#include "Phoenix.Sim.RTS/Projectiles/FeatureProjectile.h"
#include "Phoenix.Sim.RTS/Selection/FeatureSelection.h"
#include "Phoenix.Sim.RTS/Units/FeatureUnit.h"
#include "Phoenix.Sim.Script/FeatureScript.h"
#include "Phoenix.Sim.Steering/FeatureSteering.h"
#include "Phoenix.Sim.Strings/FeatureString.h"
#include "Phoenix.Sim.Tasks/FeatureTask.h"
#include "Sessions/SessionInstance.h"

using namespace Phoenix;

void SessionModule::Register(ServiceContainerBuilder& builder)
{
    IAppModule::Register(builder);
}

void SessionModule::Initialize(ModuleInitContext& context)
{
    IAppModule::Initialize(context);

    auto app = GetApplication();

    auto thisSP = std::static_pointer_cast<SessionModule>(shared_from_this());

    auto sessionDriver = app->GetService<SessionDriverService>();
    sessionDriver->SessionCreated.AddSP(thisSP, &SessionModule::OnSessionInstanceCreated);
    sessionDriver->SessionDestroyed.AddSP(thisSP, &SessionModule::OnSessionInstanceDestroyed);
}

void SessionModule::Load(ModuleLoadContext& context)
{
    IAppModule::Load(context);

    CreateInitialSession();
}

void SessionModule::Shutdown()
{
    auto app = GetApplication();

    auto sessionDriver = app->GetService<SessionDriverService>();
    sessionDriver->SessionCreated.RemoveAll(this);
    sessionDriver->SessionDestroyed.RemoveAll(this);

    IAppModule::Shutdown();
}

SessionHandle SessionModule::CreateInitialSession()
{
    auto app = GetApplication();

    auto sessionDriverService = app->ResolveService<SessionDriverService>();
    auto& sessionDriver = sessionDriverService->GetSessionDriver();

    auto serviceContainerBuilder = std::make_shared<ServiceContainerBuilder>();

    // Register features
    // ReSharper disable CppExpressionWithoutSideEffects
    serviceContainerBuilder->Register<FeatureString>().AsInterfaces();
    serviceContainerBuilder->Register<FeatureDebug>().AsInterfaces();
    serviceContainerBuilder->Register<Blackboard::FeatureBlackboard>().AsInterfaces();
    serviceContainerBuilder->Register<LDS::FeatureLDS>().AsInterfaces();
    serviceContainerBuilder->Register<ECS::FeatureECS>().AsInterfaces();
    serviceContainerBuilder->Register<Tasks::FeatureTask>().AsInterfaces();
    serviceContainerBuilder->Register<Pathfinding::FeatureNavigation>().AsInterfaces();
    serviceContainerBuilder->Register<Physics::FeaturePhysics>().AsInterfaces();
    serviceContainerBuilder->Register<Steering::FeatureSteering>().AsInterfaces();
    serviceContainerBuilder->Register<FeatureLua>().AsInterfaces();
    serviceContainerBuilder->Register<FeatureScript>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::FeatureUnit>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::FeatureAbilities>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::FeatureEffects>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::FeatureOrders>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::FeatureSelection>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::FeatureProjectiles>().AsInterfaces();

    // Register ability handlers
    serviceContainerBuilder->Register<RTS::MoveAbilityHandler>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::AttackAbilityHandler>().AsInterfaces();

    // Register effect handlers
    serviceContainerBuilder->Register<RTS::EffectDamageHandler>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::EffectLaunchProjectileHandler>().AsInterfaces();

    // Register response handlers
    serviceContainerBuilder->Register<RTS::ResponseHandlerBase>().AsInterfaces();
    serviceContainerBuilder->Register<RTS::ResponseDamageHandler>().AsInterfaces();

    SessionCtorArgs sessionArgs;
    sessionArgs.DataDirectory = "./Data";
    sessionArgs.ConfigName = "DefaultSession";
    sessionArgs.ServiceContainerBuilder = serviceContainerBuilder;

    SessionHandle handle = sessionDriver.CreateSession(sessionArgs);

    if (auto session = sessionDriver.FindInstance(handle))
    {
        auto thisSP = std::static_pointer_cast<SessionModule>(shared_from_this());
        session->WorldInstanceCreated.AddSP(thisSP, &SessionModule::OnWorldInstanceCreated);
        session->WorldInstanceDestroyed.AddSP(thisSP, &SessionModule::OnWorldInstanceDestroyed);

        session->Initialize();
        session->Start();
    }

    return handle;
}

void SessionModule::OnSessionInstanceCreated(SessionInstance* instance)
{
    auto thisSP = std::static_pointer_cast<SessionModule>(shared_from_this());
    instance->WorldInstanceCreated.AddSP(thisSP, &SessionModule::OnWorldInstanceCreated);
    instance->WorldInstanceDestroyed.AddSP(thisSP, &SessionModule::OnWorldInstanceDestroyed);
}

void SessionModule::OnSessionInstanceDestroyed(SessionInstance* instance)
{
    instance->WorldInstanceCreated.RemoveAll(this);
    instance->WorldInstanceDestroyed.RemoveAll(this);
}

void SessionModule::OnWorldInstanceCreated(WorldInstance* instance)
{
}

void SessionModule::OnWorldInstanceDestroyed(WorldInstance* instance)
{

}
