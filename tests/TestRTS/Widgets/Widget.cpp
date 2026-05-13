#include "Widget.h"

const std::string& Widget::GetId() const
{
    return Id;
}

Widget* Widget::GetParent() const
{
    return Parent.lock().get();
}

void Widget::Initialize()
{
}

void Widget::Render()
{
}
