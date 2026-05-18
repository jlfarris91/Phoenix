#include "MenuUtility.h"

std::string Phoenix::UI::JoinMenuPaths(const std::string& parent, const std::string& child)
{
    return parent.empty() ? child : (parent + "." + child);
}