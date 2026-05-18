#include "ImGuiWidgetContext.h"

#include "Object.h"

using namespace Phoenix;
using namespace Phoenix::UI;

std::shared_ptr<IObject> ImGuiWidgetContext::GetObject(const std::string& typeId) const
{
    for (const auto& object : Objects)
    {
        if (object->GetTypeDescriptor().GetQualifiedName() == typeId)
        {
            return object;
        }
    }
    return {};
}

const std::vector<std::shared_ptr<IObject>>& ImGuiWidgetContext::GetObjects() const
{
    return Objects;
}

void ImGuiWidgetContext::AddObject(const std::shared_ptr<IObject>& object)
{
    if (std::ranges::find(Objects, object) == Objects.end())
    {
        Objects.push_back(object);
    }
}

bool ImGuiWidgetContext::RemoveObject(const std::shared_ptr<IObject>& object)
{
    auto iter = std::ranges::find(Objects, object);
    if (iter != Objects.end())
    {
        Objects.erase(iter);
        return true;
    }
    return false;
}
