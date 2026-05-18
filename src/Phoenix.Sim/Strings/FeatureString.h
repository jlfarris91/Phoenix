#pragma once

#include "StringTable.h"
#include "Phoenix.Sim/Features.h"
#include "Phoenix.Sim/SessionFwd.h"

namespace Phoenix
{
    struct PHOENIX_SIM_API FeatureStringDynamicBlock : BlockBufferBlock
    {
        PHX_DECLARE_BLOCK_WITH_ALLOC(FeatureStringDynamicBlock)
        {
            uint32 MaxNumStrings = 0;
            uint32 MaxBufferCapacity = 0;
        };

        FixedStringTable StringTable;
    };

    class PHOENIX_SIM_API FeatureString : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureString) {}

    public:

        // Checks if a string with the given name exists in the session table.
        static bool Contains(SessionConstRef session, const FName& name);

        // Checks if a string with the given name exists in the world's string table.
        static bool Contains(WorldConstRef world, const FName& name);

        // Returns the character string for a given name.
        // Returns nullptr if the name doesn't exist in the table.
        static const char* Get(SessionConstRef session, const FName& name);

        // Returns the character string for a given name.
        // Returns nullptr if the name doesn't exist in the table.
        static const char* Get(WorldConstRef world, const FName& name);

        // Stores a character string in the table and returns a pointer to the stored string.
        static const char* Store(SessionRef session, const char* str, uint32 len);

        // Stores a character string in the table and returns a pointer to the stored string.
        static const char* Store(WorldRef world, const char* str, uint32 len);

        // Stores a character string in the table with the given name and returns a pointer to the stored string.
        static const char* StoreAs(SessionRef session, const char* str, uint32 len, const FName& name);

        // Stores a character string in the table with the given name and returns a pointer to the stored string.
        static const char* StoreAs(WorldRef world, const char* str, uint32 len, const FName& name);

    protected:

        void OnSessionLayout(const SessionLayoutContext& context, BlockBufferConfigBuilder& builder) override;
        void OnWorldLayout(const WorldLayoutContext& context, BlockBufferConfigBuilder& builder) override;
    };
}
