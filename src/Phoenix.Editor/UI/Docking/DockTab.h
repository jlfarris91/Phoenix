#pragma once

#include <memory>
#include "Phoenix/Reflection/Registration.h"

#include "DockTabId.h"

namespace Phoenix
{
    class IObject;
}

namespace Phoenix::UI
{
    class DockTab : public std::enable_shared_from_this<DockTab>
    {
        PHX_DECLARE_TYPE_INTERFACE(DockTab)

    public:

        DockTab(const DockTabId& id);
        virtual ~DockTab() = default;

        const DockTabId& GetId() const;

        std::shared_ptr<IObject> GetContext() const;
        void SetContext(const std::shared_ptr<IObject>& context);

        bool IsOpened() const;

        // Called by the DockManager
        virtual void OnOpened();
        virtual void OnClosed();

    private:
        DockTabId Id;
        std::shared_ptr<IObject> Context;
        bool bIsOpened = false;
    };
}
