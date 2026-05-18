#include "MenuAssembler.h"

#include "Menu.h"
#include "MenuEntry.h"
#include "MenuManager.h"

using namespace Phoenix::UI;

void MenuAssembler::Initialize(const std::shared_ptr<IMenuManager>& menuManager)
{
    MenuManager = menuManager;
}

void MenuAssembler::AssembleMenu(Menu& menu, const Menu& source)
{
    auto menuManager = MenuManager.lock();
    if (!menuManager)
    {
        return;
    }

    std::vector<MenuSection> remainingSections;

    Menu constructedSections;
    constructedSections.Initialize(menuManager, "TempAssembleMenu", {}, &menu.Context);

    for (const MenuSection& sourceSection : source.Sections)
    {
        std::vector<MenuSection> generatedSections;
        generatedSections.push_back(sourceSection);

        while (!generatedSections.empty())
        {
            if (generatedSections.front().Construct)
            {
                generatedSections.front().Construct(constructedSections);

                auto insertIter = remainingSections.begin();
                for (MenuSection& constructedSection : constructedSections.Sections)
                {
                    if (constructedSection.InsertPosition.IsDefault())
                    {
                        constructedSection.InsertPosition = generatedSections.front().InsertPosition;
                    }

                    insertIter = remainingSections.insert(insertIter, constructedSection);
                }
            }
            else
            {
                remainingSections.push_back(generatedSections.front());
            }

            generatedSections.erase(generatedSections.begin());
        }
    }

    while (!remainingSections.empty())
    {
        bool handledAny = false;
        for (size_t i = 0; i < remainingSections.size(); ++i)
        {
            MenuSection& remainingSection = remainingSections[i];

            MenuSection* section = menu.FindSection(remainingSection.Name);
            if (!section)
            {
                int32_t destIndex = menu.FindInsertIndex(remainingSection);
                if (destIndex != -1)
                {
                    if (static_cast<size_t>(destIndex) < menu.Sections.size())
                    {
                        menu.Sections.insert(menu.Sections.begin() + destIndex, {});
                        section = &menu.Sections[destIndex];
                    }
                    else
                    {
                        menu.Sections.emplace_back();
                        section = &menu.Sections.back();
                    }
                    section->InitializeFrom(remainingSection, menu.Context);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if (!section->Label.HasValue() && remainingSection.Label.HasValue())
                {
                    section->Label = remainingSection.Label;
                }
                if (!section->Construct && remainingSection.Construct)
                {
                    section->Construct = remainingSection.Construct;
                }
            }

            AssembleMenuSection(*section, remainingSection);
            remainingSections.erase(remainingSections.begin() + i);
            handledAny = true;
            break;
        }

        if (!handledAny)
        {
            // TODO (jfarris): Log out failure to insert sections
            break;
        }
    }
}

void MenuAssembler::AssembleMenuHierarchy(Menu& menu, const std::vector<std::shared_ptr<Menu>>& hierarchy)
{
    for (const auto& sourceMenu : hierarchy)
    {
        AssembleMenu(menu, *sourceMenu.get());
    }
}

void MenuAssembler::AssembleMenuSection(MenuSection& section, const MenuSection& source)
{
    std::vector<MenuEntry> remainingEntries = source.Entries;

    while (!remainingEntries.empty())
    {
        bool handledAny = false;
        for (size_t i = 0; i < remainingEntries.size(); ++i)
        {
            MenuEntry& entry = remainingEntries[i];
            int32_t destIndex = section.FindInsertIndex(entry);
            if (destIndex != -1)
            {
                if (static_cast<size_t>(destIndex) < section.Entries.size())
                {
                    section.Entries.insert(section.Entries.begin() + destIndex, entry);
                }
                else
                {
                    section.Entries.emplace_back(entry);
                }
                remainingEntries.erase(remainingEntries.begin() + i);
                handledAny = true;
                break;
            }
        }
        if (!handledAny)
        {
            // TODO (jfarris): Log out failure to insert
            break;
        }
    }
}
