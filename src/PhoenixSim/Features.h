#pragma once

#include <map>

#include "PhoenixSim/Containers/Array.h"
#include "PhoenixSim/Reflection.h"
#include "PhoenixSim/Services/Service.h"
#include "PhoenixSim/Worlds.h"

namespace Phoenix
{
    struct IDebugState;
    struct IDebugRenderer;
    class Session;
}

#define PHX_FEATURE_BEGIN(feature) \
        PHX_DECLARE_TYPE_WITH_DESCRIPTOR_BEGIN(feature, Phoenix::FeatureDefinition) \
            PHX_REGISTER_BASE(Phoenix::IFeature)

#define PHX_FEATURE_END() \
        PHX_DECLARE_TYPE_WITH_DESCRIPTOR_END(Phoenix::FeatureDefinition) \
        const FeatureDefinition& GetFeatureDefinition() override { return STypeDescriptor::StaticGet(); }

#define FEATURE_SESSION_BLOCK(block) definition.RegisterSessionBlock<block>();
#define FEATURE_WORLD_BLOCK(block) definition.RegisterWorldBlock<block>();
#define FEATURE_CHANNEL(...) definition.RegisterChannel(__VA_ARGS__);

namespace Phoenix
{
    struct Action;

    struct FeatureChannels
    {
#define PHX_DECLARE_CHANNEL(name) static constexpr FName name = #name##_n

        // Session
        PHX_DECLARE_CHANNEL(PreUpdate);
        PHX_DECLARE_CHANNEL(Update);
        PHX_DECLARE_CHANNEL(PostUpdate);
        PHX_DECLARE_CHANNEL(PreHandleAction);
        PHX_DECLARE_CHANNEL(HandleAction);
        PHX_DECLARE_CHANNEL(PostHandleAction);

        // World
        PHX_DECLARE_CHANNEL(WorldInitialize);
        PHX_DECLARE_CHANNEL(WorldShutdown);
        PHX_DECLARE_CHANNEL(PreWorldUpdate);
        PHX_DECLARE_CHANNEL(WorldUpdate);
        PHX_DECLARE_CHANNEL(PostWorldUpdate);
        PHX_DECLARE_CHANNEL(PreHandleWorldAction);
        PHX_DECLARE_CHANNEL(HandleWorldAction);
        PHX_DECLARE_CHANNEL(PostHandleWorldAction);

        PHX_DECLARE_CHANNEL(DebugRender);

#undef PHX_DECLARE_CHANNEL
    };

    struct PHOENIX_SIM_API FeatureUpdateArgs
    {
        simtime_t SimTime = 0;
        uint32 StepHz = 0;
    };

    struct PHOENIX_SIM_API FeatureActionArgs
    {
        simtime_t SimTime = 0;
        Action Action;
    };
    
    class PHOENIX_SIM_API IFeature : public IService
    {
        PHX_DECLARE_TYPE_BEGIN(IFeature)
            PHX_REGISTER_BASE(IService)
        PHX_DECLARE_TYPE_END()

    public:

        // Gets the name of the feature.
        virtual FName GetName() const;

        virtual const struct FeatureDefinition& GetFeatureDefinition() = 0;

        // Called when a new world is created and gives the feature a chance to initialize.
        virtual void OnWorldInitialize(WorldRef world);

        // Called when a world is about to be released and gives the feature a chance to clean up.
        virtual void OnWorldShutdown(WorldRef world);

        // Called once per session step, before OnUpdate.
        virtual void OnPreUpdate(const FeatureUpdateArgs& args);

        // Called once per session step.
        virtual void OnUpdate(const FeatureUpdateArgs& args);

        // Called once per session step, after OnUpdate.
        virtual void OnPostUpdate(const FeatureUpdateArgs& args);

        // Called once per action sent to the session, before OnHandleAction.
        virtual bool OnPreHandleAction(const FeatureActionArgs& args);
 
        // Called once per action sent to the session.
        virtual bool OnHandleAction(const FeatureActionArgs& args);

        // Called once per action sent to the session, after OnHandleAction.
        virtual bool OnPostHandleAction(const FeatureActionArgs& args);

        // Called once per session step, per world, before OnWorldUpdate.
        virtual void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args);

        // Called once per session step, per world.
        virtual void OnWorldUpdate(WorldRef world, const FeatureUpdateArgs& args);

        // Called once per session step, per world, after OnWorldUpdate.
        virtual void OnPostWorldUpdate(WorldRef world, const FeatureUpdateArgs& args);

        // Called once per action sent to a specific world, before OnHandleWorldAction.
        virtual bool OnPreHandleWorldAction(WorldRef world, const FeatureActionArgs& args);

        // Called once per action sent to a specific world.
        virtual bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& args);

        // Called once per action sent to a specific world, after OnHandleWorldAction.
        virtual bool OnPostHandleWorldAction(WorldRef world, const FeatureActionArgs& args);

        // Gives the feature the ability to render debug information for a given world.
        virtual void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer);

    protected:

        friend class Session;

        nlohmann::json Config;
    };

    typedef IFeature* FeaturePtr;
    typedef const IFeature* FeatureConstPtr;
    typedef TSharedPtr<IFeature> FeatureSharedPtr;
    typedef TSharedPtr<const IFeature> FeatureSharedConstPtr;

    enum class EFeatureInsertPosition : uint8
    {
        Default,
        Before,
        After,
    };

    struct PHOENIX_SIM_API FeatureInsertPosition
    {
        static const FeatureInsertPosition Default;
        FName FeatureName;
        EFeatureInsertPosition RelativePosition = EFeatureInsertPosition::Default;
    };

    struct PHOENIX_SIM_API FeatureChannelInsertArgs
    {
        FeatureChannelInsertArgs() = default;

        FeatureChannelInsertArgs(
            const FName& channelName,
            const FeatureInsertPosition& position = {});

        FName Channel;
        FeatureInsertPosition InsertPosition;
    };

    struct PHOENIX_SIM_API FeatureSetCtorArgs
    {
        TArray2<FeatureSharedPtr> Features;
    };

    struct PHOENIX_SIM_API FeatureDefinition : TypeDescriptor
    {
        BlockBuffer::CtorArgs SessionBlocks;
        BlockBuffer::CtorArgs WorldBlocks;
        TArray<FeatureChannelInsertArgs> Channels;
        TArray<FName> DependentFeatures;

        template <class TBlock>
        void RegisterSessionBlock()
        {
            SessionBlocks.RegisterBlock<TBlock>();
        }

        template <class TBlock>
        void RegisterWorldBlock()
        {
            WorldBlocks.RegisterBlock<TBlock>();
        }

        void RegisterChannel(FName channel, const FeatureInsertPosition& insertPosition = FeatureInsertPosition::Default)
        {
            Channels.emplace_back(channel, insertPosition);
        }
    };

    class PHOENIX_SIM_API FeatureSet
    {
    public:

        FeatureSet(const FeatureSetCtorArgs& args);

        FeatureSharedPtr GetFeature(const FName& name) const;

        template <class TFeature>
        TSharedPtr<TFeature> GetFeature(const FName& name) const
        {
            return std::static_pointer_cast<TFeature>(GetFeature(name));
        }

        template <class TFeature>
        TSharedPtr<TFeature> GetFeature() const
        {
            return GetFeature<TFeature>(TFeature::StaticTypeName);
        }

        TArray2<FeatureSharedPtr> GetFeatures() const;

        // Gets an array containing all the names of the channels.
        TArray2<FName> GetChannelNames() const;
        
        TArray2<FeatureSharedPtr> GetChannel(const FName& channelName) const;
        const TArray2<FeatureSharedPtr>& GetChannelRef(const FName& channelName) const;

        template <class TCallback>
        void ForEachFeatureInChannel(const FName& channelName, const TCallback& callback)
        {
            auto&& channelEntry = Channels.find(channelName);
            if (channelEntry == Channels.end())
                return;

            const auto& channel = channelEntry->second;
            for (auto&& feature : channel)
            {
                if (InvokeForEachCallbackNoIndex(callback, *feature))
                {
                    break;
                }
            }
        }

    private:
        
        void RegisterFeatureChannels(const TArray2<FeatureSharedPtr>& featureDefs);

        static int32 FindChannelInsertIndex(
            const TArray2<FeatureSharedPtr>& channelFeatures,
            const FeatureInsertPosition& insertPosition);

        TMap<FName, FeatureSharedPtr> Features;
        TMap<FName, TArray2<FeatureSharedPtr>> Channels;
    };
}
