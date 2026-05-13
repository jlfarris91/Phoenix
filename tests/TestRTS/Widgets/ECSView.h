#pragma once

#include "WorldViewWidget.h"

class ECSView : public WorldViewWidget
{
protected:

    void Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world) override;

private:
    static void RenderSystems(Phoenix::SessionConstRef session);
    static void RenderArchetypes(Phoenix::WorldConstRef world);
    static void RenderEntities(Phoenix::WorldConstRef world);
    static void RenderTags(Phoenix::WorldConstRef world);
    static void RenderGroups(Phoenix::WorldConstRef world);
};
