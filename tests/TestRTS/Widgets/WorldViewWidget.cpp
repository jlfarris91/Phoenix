#include "WorldViewWidget.h"

#include "PhoenixSim/Session.h"

using namespace Phoenix;

void WorldViewWidget::Render()
{
    Widget::Render();

    const Session* sessionPtr = nullptr;
    if (!sessionPtr)
    {
        return;
    }

    const World* worldViewPtr = nullptr;
    if (!worldViewPtr)
    {
        return;
    }

    Render(*sessionPtr, *worldViewPtr);
}

