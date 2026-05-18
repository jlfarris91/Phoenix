#include "ImGuiWidgetFactory.h"

#include <algorithm>

using namespace Phoenix::UI;

void ImGuiWidgetFactoryService::RegisterWidgetFactory(const std::shared_ptr<IImGuiWidgetFactory>& factory)
{
    if (std::ranges::find(WidgetFactories, factory) == WidgetFactories.end())
    {
        WidgetFactories.push_back(factory);
    }
}

bool ImGuiWidgetFactoryService::UnregisterWidgetFactory(const std::shared_ptr<IImGuiWidgetFactory>& factory)
{
    return std::erase(WidgetFactories, factory) != 0;
}

bool ImGuiWidgetFactoryService::CanCreateWidget(const std::string& widgetType) const
{
    return std::ranges::any_of(WidgetFactories, [&](const auto& factory)
    {
        return factory->CanCreateWidget(widgetType);
    });
}

std::shared_ptr<ImGuiWidget> ImGuiWidgetFactoryService::CreateWidget(
    const std::string& widgetType,
    const ImGuiWidgetContext& context)
{
    for (const auto& factory : WidgetFactories)
    {
        if (!factory->CanCreateWidget(widgetType))
        {
            continue;
        }
        if (auto widget = factory->CreateWidget(widgetType, context))
        {
            return widget;
        }
    }
    return {};
}

std::shared_ptr<IImGuiWidgetFactory> ImGuiWidgetFactoryService::FindFactoryForWidgetType(
    const std::string& widgetType) const
{
    for (const auto& factory : WidgetFactories)
    {
        if (factory->CanCreateWidget(widgetType))
        {
            return factory;
        }
    }
    return {};
}
