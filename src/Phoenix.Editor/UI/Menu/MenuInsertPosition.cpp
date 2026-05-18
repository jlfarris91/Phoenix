#include "MenuInsertPosition.h"

using namespace Phoenix::UI;

MenuInsertPosition::MenuInsertPosition(const std::string& name, EMenuInsertPosition position)
    : Name(name)
    , Position(position)
{
}

bool MenuInsertPosition::IsDefault() const
{
    return Position == EMenuInsertPosition::Default;
}

bool MenuInsertPosition::IsBeforeOrAfter() const
{
    return Position == EMenuInsertPosition::Before || Position == EMenuInsertPosition::After;
}

bool MenuInsertPosition::operator==(const MenuInsertPosition& other) const
{
    return Name == other.Name && Position == other.Position;
}

bool MenuInsertPosition::operator!=(const MenuInsertPosition& other) const
{
    return !(*this == other);
}
