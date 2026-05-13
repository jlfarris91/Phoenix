#pragma once

#include <memory>
#include <vector>

#include "Widget.h"

class PanelWidget : public Widget
{
public:

    Widget& AddWidget(const std::unique_ptr<Widget>& widget);

    bool RemoveWidgetAt(uint32_t index);

    bool RemoveWidget(const Widget& widget);

    const std::vector<std::unique_ptr<Widget>>& GetChildren() const;

protected:

    std::vector<std::unique_ptr<Widget>> Children;
};
