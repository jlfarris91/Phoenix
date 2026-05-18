#include "Phoenix.Sim.RTS/TargetFiltering/TargetFiltering.h"

#include "Phoenix.Sim/ECS/FeatureECS.h"

#include "Phoenix.Sim.RTS/Data/DataTargetFilter.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::ECS;
using namespace Phoenix::RTS;
using namespace Phoenix::RTS::Data;

bool TargetFiltering::PassesTagFilter(const World& world, const TagFilter& filter, const EntityId& target)
{
    bool hasAllRequired = true;

    for (const TagPtr& tagPtr : filter.Required)
    {
        if (!tagPtr.IsValid())
        {
            continue;
        }

        if (!FeatureECS::HasTag(world, target, tagPtr.GetObjectId()))
        {
            hasAllRequired = false;
            break;
        }
    }

    if (!hasAllRequired)
    {
        return false;
    }

    bool hasExcluded = false;

    for (const TagPtr& tagPtr : filter.Excluded)
    {
        if (!tagPtr.IsValid())
        {
            continue;
        }

        if (FeatureECS::HasTag(world, target, tagPtr.GetObjectId()))
        {
            hasExcluded = true;
            break;
        }
    }

    if (hasExcluded)
    {
        return false;
    }

    return true;
}

bool TargetFiltering::PassesTargetFilter(const World& world, const TargetFilter& filter, const EntityId& source, const EntityId& target)
{
    if (!PassesTagFilter(world, filter.TagFilter, target))
    {
        return false;
    }

    return true;
}
