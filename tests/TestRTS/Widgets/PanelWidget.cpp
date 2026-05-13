#include "PanelWidget.h"

Widget& PanelWidget::AddWidget(const std::unique_ptr<Widget>& widget)
{
    Children.push_back(std::move(widget));
    return *Children.back();
}

bool PanelWidget::RemoveWidgetAt(uint32_t index)
{
    if (index < Children.size())
    {
        Children.erase(Children.begin() + index);
        return true;
    }
    return false;
}

bool PanelWidget::RemoveWidget(const Widget& widget)
{
    uint32_t index = 0;
    for (auto&& child : Children)
    {
        if (child.get() == &widget)
        {
            return RemoveWidgetAt(index);
        }
        ++index;
    }
    return false;
}

const std::vector<std::unique_ptr<Widget>>& PanelWidget::GetChildren() const
{
    return Children;
}
