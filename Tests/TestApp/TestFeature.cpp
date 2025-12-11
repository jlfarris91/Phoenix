
#include "TestFeature.h"

#include "Flags.h"

#include "FeatureECS.h"
#include "FeaturePhysics.h"

#include "BodyComponent.h"
#include "SteeringComponent.h"

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

bool TestFeature::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    if (action.Action.Verb == "spawn_entity"_n)
    {
        for (uint32 i = 0; i < action.Action.Data[4].UInt32; ++i)
        {
            EntityId entityId = FeatureECS::AcquireEntity(world, action.Action.Data[0].Name);
            if (entityId == EntityId::Invalid)
                break;

            TransformComponent* transformComp = FeatureECS::GetComponent<TransformComponent>(world, entityId);
            PHX_ASSERT(transformComp);
            transformComp->Transform.Position.X = action.Action.Data[1].Distance;
            transformComp->Transform.Position.Y = action.Action.Data[2].Distance;
            transformComp->Transform.Rotation = action.Action.Data[3].Degrees;

            BodyComponent* bodyComp = FeatureECS::GetComponent<BodyComponent>(world, entityId);
            bodyComp->CollisionMask = 1;
            bodyComp->Radius = 0.6; // Lancer :)
            bodyComp->InvMass = OneDivBy<Value>(1.0f);
            bodyComp->LinearDamping = 10.f;
            SetFlagRef(bodyComp->Flags, EBodyFlags::Awake, true);

            Color color;
            color.R = rand() % 255;
            color.G = rand() % 255;
            color.B = rand() % 255;
            FeatureECS::SetBlackboardValue(world, entityId, "actor_tint"_n, color);

            Steering::SteeringComponent* steeringComp = FeatureECS::GetComponent<Steering::SteeringComponent>(world, entityId);
            steeringComp->AvoidanceRadius = bodyComp->Radius * 2;
            steeringComp->MaxSpeed = 50.0f;
        }

        return true;
    }

    return false;
}
