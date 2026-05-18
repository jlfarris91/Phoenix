#pragma once

#include <memory>
#include <string>
#include "Phoenix/Reflection/Registration.h"

namespace Phoenix::UI
{
    class ImGuiWidgetContext;
    class ImGuiWidget;

    class IImGuiWidgetFactory
    {
        PHX_DECLARE_TYPE_INTERFACE(IImGuiWidgetFactory)
    public:
        virtual ~IImGuiWidgetFactory() = default;

        virtual bool CanCreateWidget(const std::string& widgetType) const = 0;

        virtual std::shared_ptr<ImGuiWidget> CreateWidget(
            const std::string& widgetType,
            const ImGuiWidgetContext& context) = 0;
    };

    class ImGuiWidgetFactoryService : public IImGuiWidgetFactory
    {
        PHX_DECLARE_TYPE_DERIVED(ImGuiWidgetFactoryService, IImGuiWidgetFactory)
    public:

        void RegisterWidgetFactory(const std::shared_ptr<IImGuiWidgetFactory>& factory);
        bool UnregisterWidgetFactory(const std::shared_ptr<IImGuiWidgetFactory>& factory);

        virtual bool CanCreateWidget(const std::string& widgetType) const override;

        virtual std::shared_ptr<ImGuiWidget> CreateWidget(
            const std::string& widgetType,
            const ImGuiWidgetContext& context) override;

        std::shared_ptr<IImGuiWidgetFactory> FindFactoryForWidgetType(const std::string& widgetType) const;

    private:
        std::vector<std::shared_ptr<IImGuiWidgetFactory>> WidgetFactories;
    };
}
