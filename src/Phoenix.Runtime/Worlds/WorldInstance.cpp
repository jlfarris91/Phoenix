#include "WorldInstance.h"

namespace Phoenix
{
    WorldInstance::WorldInstance(FName id)
        : Id(id)
    {
    }

    FName WorldInstance::GetId() const
    {
        return Id;
    }

    const World* WorldInstance::GetWorldView() const
    {
        return DoubleBuffer.GetWorldView();
    }

    double WorldInstance::GetUpdateRate() const
    {
        return DoubleBuffer.GetUpdateRate();
    }

    uint32 WorldInstance::GetAccumulatedDirtyPageCount() const
    {
        return DoubleBuffer.GetAccumulatedDirtyPageCount();
    }

    void WorldInstance::Sink()
    {
        DoubleBuffer.Sink();
    }

    void WorldInstance::OnSimUpdate(WorldConstRef world)
    {
        DoubleBuffer.OnSimUpdate(world);
    }
}
