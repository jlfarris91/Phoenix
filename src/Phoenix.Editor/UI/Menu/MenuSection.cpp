#include "MenuSection.h"

using namespace Phoenix;

void UI::MenuSection::Initialize(
    const std::string& name,
    const Attribute<std::string>& label,
    const MenuInsertPosition& insertPosition)
{
    Name = name;
    Label = label;
    InsertPosition = insertPosition;
}

std::string_view UI::MenuSection::GetName() const
{
    return Name;
}

const std::vector<UI::MenuEntry>& UI::MenuSection::GetEntries() const
{
    return Entries;
}

std::string UI::MenuSection::GetLabel() const
{
    return Label.GetValue();
}

void UI::MenuSection::SetLabel(const Attribute<std::string>& label)
{
    Label = label;
}

UI::MenuEntry& UI::MenuSection::AddEntry(const MenuEntry& entry)
{
    if (!entry.GetName().empty())
    {
        if (MenuEntry* existingEntry = FindEntry(entry.GetName()))
        {
            *existingEntry = entry;
            return *existingEntry;
        }
    }

    return Entries.emplace_back(entry);
}

UI::MenuEntry& UI::MenuSection::AddEntry(
    const std::shared_ptr<const CommandInfo>& command,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const Attribute<Icon>& icon)
{
    return AddEntry(MenuEntry::InitMenuEntry(command->GetName(), command, label, tooltip, icon));
}

UI::MenuEntry& UI::MenuSection::AddSeparator(const std::string& name)
{
    return AddEntry(MenuEntry::InitSeparator(name));
}

UI::MenuEntry& UI::MenuSection::AddSubMenu(
    const std::string& name,
    const Attribute<std::string>& label,
    const Attribute<std::string>& tooltip,
    const NewMenuFunc& newMenuFunc,
    const MenuAction& action,
    EUIActionType actionType,
    const Attribute<Icon>& icon)
{
    return AddEntry(MenuEntry::InitSubMenu(name, label, tooltip, newMenuFunc, action, actionType, icon));
}

UI::MenuEntry* UI::MenuSection::FindEntry(const std::string& name)
{
    for (auto& entry : Entries)
    {
        if (entry.GetName() == name)
        {
            return &entry;
        }
    }
    return nullptr;
}

const UI::MenuEntry* UI::MenuSection::FindEntry(const std::string& name) const
{
    for (const auto& entry : Entries)
    {
        if (entry.GetName() == name)
        {
            return &entry;
        }
    }
    return nullptr;
}

void UI::MenuSection::InitializeFrom(const MenuSection& source, const MenuContext& context)
{
    Name = source.Name;
    Label = source.Label;
    InsertPosition = source.InsertPosition;
    Construct = source.Construct;
    Context = context;
}

int32_t UI::MenuSection::FindInsertIndex(const MenuEntry& entry) const
{
    const MenuInsertPosition& insertPosition = entry.GetInsertPosition();

    if (insertPosition.IsDefault())
    {
        return static_cast<int32_t>(Entries.size());
    }

    if (insertPosition.Position == EMenuInsertPosition::First)
    {
        for (size_t i = 0; i < Entries.size(); ++i)
        {
            if (Entries[i].GetInsertPosition() != insertPosition)
            {
                return static_cast<int32_t>(i);
            }
        }

        return static_cast<int32_t>(Entries.size());
    }

    int32_t destIndex = -1;
    for (const auto& existingEntry : Entries)
    {
        if (existingEntry.GetName() == insertPosition.Name)
        {
            destIndex = static_cast<int32_t>(&existingEntry - Entries.data());
            break;
        }
    }

    if (destIndex == -1)
    {
        return -1;
    }

    if (insertPosition.Position == EMenuInsertPosition::After)
    {
        ++destIndex;
    }

    for (size_t i = destIndex; i < Entries.size(); ++i)
    {
        if (Entries[i].GetInsertPosition() != insertPosition)
        {
            return static_cast<int32_t>(i);
        }
    }

    return static_cast<int32_t>(Entries.size());
}
