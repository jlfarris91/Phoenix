#pragma once

#include <Phoenix/Name.h>
#include <Phoenix.Sim/WorldsFwd.h>

#include "WorldDoubleBuffer.h"

namespace Phoenix
{
    class WorldInstance
    {
    public:
        explicit WorldInstance(FName id);

        FName GetId() const;

        // Returns the stable world view for the game thread. Null until the first sim step.
        const World* GetWorldView() const;

        double GetUpdateRate() const;
        uint32 GetAccumulatedDirtyPageCount() const;

    private:

        // For calling OnSimUpdate & Sink
        friend class SessionInstance;

        // Called by the game thread each frame to advance the double buffer.
        void Sink();

        // Called by the sim thread after each world step completes.
        void OnSimUpdate(WorldConstRef world);

        FName             Id;
        WorldDoubleBuffer DoubleBuffer;
    };
}
