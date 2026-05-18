#include "DockTab.h"

Phoenix::UI::DockTab::DockTab(const DockTabId& id)
    : Id(id)
{
}

const Phoenix::UI::DockTabId& Phoenix::UI::DockTab::GetId() const
{
    return Id;
}

std::shared_ptr<Phoenix::IObject> Phoenix::UI::DockTab::GetContext() const
{
    return Context;
}

void Phoenix::UI::DockTab::SetContext(const std::shared_ptr<IObject>& context)
{
    Context = context;
}

bool Phoenix::UI::DockTab::IsOpened() const
{
    return bIsOpened;
}

void Phoenix::UI::DockTab::OnOpened()
{
    bIsOpened = true;
}

void Phoenix::UI::DockTab::OnClosed()
{
    bIsOpened = false;
}
