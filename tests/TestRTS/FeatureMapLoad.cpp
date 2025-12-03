
#include "FeatureMapLoad.h"

#include "BodyComponent.h"
#include "FeatureLDS.h"
#include "SteeringComponent.h"
#include "Json/JsonDataSource.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;

void FeatureMapLoad::Initialize()
{
    IFeature::Initialize();

    TSharedPtr<FeatureLDS> featureLDS = Session->GetFeature<FeatureLDS>();

    TSharedPtr<Catalog> sessionCatalog = featureLDS->GetStaticSessionCatalog();

    PHXString projectDir = Session->GetProjectDirectory();

    Json::JsonDataSource dataSource;
    // dataSource.LoadFromDirectory(sessionDir)
}

void FeatureMapLoad::OnWorldInitialize(WorldRef world)
{
    IFeature::OnWorldInitialize(world);

    FName worldName = world.GetName();

    
}
