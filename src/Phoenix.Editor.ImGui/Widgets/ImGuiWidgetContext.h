#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Phoenix
{
    class IObject;
}

namespace Phoenix::UI
{
    class ImGuiWidgetContext
    {
    public:

        std::shared_ptr<IObject> GetObject(const std::string& typeId) const;

        const std::vector<std::shared_ptr<IObject>>& GetObjects() const;

        void AddObject(const std::shared_ptr<IObject>& object);

        bool RemoveObject(const std::shared_ptr<IObject>& object);

    private:

        std::vector<std::shared_ptr<IObject>> Objects;
    };
}
