
#include "FeatureMapLoad.h"

#include <PhoenixSim/LDS/FeatureLDS.h>
#include <PhoenixSim/LDS/Json/JsonDataSource.h>
#include <PhoenixPhysics/BodyComponent.h>
#include <PhoenixSteering/SteeringComponent.h>

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

void FeatureMapLoad::Initialize(const TSharedPtr<Phoenix::Session>& session)
{
    IFeature::Initialize(session);

    TSharedPtr<FeatureLDS> featureLDS = Session->GetFeature<FeatureLDS>();

    TSharedPtr<Catalog> sessionCatalog = featureLDS->GetStaticSessionCatalog();

    PHXString projectDir = Session->GetDataDirectory();

    Json::JsonDataSource dataSource;
    // dataSource.LoadFromDirectory(sessionDir)
}

void FeatureMapLoad::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);
}
