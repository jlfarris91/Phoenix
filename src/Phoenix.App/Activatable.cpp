#include "Activatable.h"

bool Phoenix::IActivatable::CanActivate() const
{
    return true;
}

void Phoenix::IActivatable::Activate()
{
    if (IsActivated())
    {
        return;
    }

    bIsActivated = true;
    OnActivated();
}

void Phoenix::IActivatable::Deactivate()
{
    if (!IsActivated())
    {
        return;
    }

    OnDeactivated();
    bIsActivated = false;
}

bool Phoenix::IActivatable::IsActivated() const
{
    return bIsActivated;
}

void Phoenix::IActivatable::SetActivated(bool value)
{
    if (value)
    {
        Activate();
    }
    else
    {
        Deactivate();
    }
}
