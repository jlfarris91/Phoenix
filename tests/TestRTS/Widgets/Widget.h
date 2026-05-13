#pragma once

#include <memory>
#include <string>

class Widget : public std::enable_shared_from_this<Widget>
{
public:

    virtual ~Widget() = default;

    const std::string& GetId() const;

    Widget* GetParent() const;

    virtual void Initialize();
    virtual void Render();

protected:

    std::string Id;
    std::weak_ptr<Widget> Parent;
};
