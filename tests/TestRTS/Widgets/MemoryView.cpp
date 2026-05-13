#include "MemoryView.h"

void MemoryView::Initialize()
{
    WorldViewWidget::Initialize();
}

void MemoryView::Render(Phoenix::SessionConstRef session, Phoenix::WorldConstRef world)
{
    MemoryTool.Draw(ImGui::GetIO().DeltaTime);
}
