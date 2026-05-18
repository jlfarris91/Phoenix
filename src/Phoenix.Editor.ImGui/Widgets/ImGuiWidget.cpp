#include "ImGuiWidget.h"

const Phoenix::UI::ImGuiWidgetContext& Phoenix::UI::ImGuiWidget::GetContext() const
{
    return Context;
}

void Phoenix::UI::ImGuiWidget::SetContext(const ImGuiWidgetContext& context)
{
    Context = context;
}

std::shared_ptr<Phoenix::IObject> Phoenix::UI::ImGuiWidget::GetContextObject(const std::string& typeId) const
{
    return Context.GetObject(typeId);
}
