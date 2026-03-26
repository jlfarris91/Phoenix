#pragma once

#include "PhoenixSim/Reflection/Registration.h"
#include "PhoenixSim/WorldsFwd.h"

namespace Phoenix
{
    class Session;
    struct SessionLayoutContext;
    struct WorldLayoutContext;
    struct BlockBufferLayoutBuilder;

    class PHOENIX_SIM_API IService : public std::enable_shared_from_this<IService>
    {
        // IService is the one type that introduces the virtual GetTypeDescriptor()
        // contract, so it cannot use PHX_DECLARE_TYPE (which injects a non-virtual
        // definition that would conflict).  Use PHX_TYPE_BODY_ directly instead.
        PHX_TYPE_BODY_(IService)

    private:
        inline static const bool _s_phx_type_init_ =
            (Phoenix::TypeRegistry::GetOrCreate<IService>(), true);

    public:

        static const TypeDescriptor& GetStaticTypeDescriptor()
        {
            return TypeRegistry::GetOrCreate<IService>();
        }

        virtual const TypeDescriptor& GetTypeDescriptor() const = 0;

        // Gets the session that this service belongs to.
        Session* GetSession() const;

        virtual void OnSessionLayout(const SessionLayoutContext& context, BlockBufferLayoutBuilder& builder);
        virtual void Initialize(const std::shared_ptr<Session>& session);
        virtual void Shutdown();

        virtual void OnWorldLayout(const WorldLayoutContext& context, BlockBufferLayoutBuilder& builder);
        virtual void OnWorldInitialize(WorldRef world);
        virtual void OnWorldShutdown(WorldRef world);

    protected:

        std::shared_ptr<Session> Session;
    };
}
