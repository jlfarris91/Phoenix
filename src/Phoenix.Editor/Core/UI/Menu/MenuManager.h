#pragma once

#include "Menu.h"
#include "MenuAssembler.h"
#include "Editor/EditorService.h"

namespace Phoenix::UI
{
    class IMenuManager : public IEditorService
    {
        PHX_DECLARE_TYPE_DERIVED(IMenuManager, IEditorService)

    public:

        virtual std::shared_ptr<Menu> RegisterMenu(const std::string& name, const std::string& parentName = {}) = 0;

        virtual std::shared_ptr<Menu> FindMenu(const std::string& name) const = 0;

        virtual std::shared_ptr<Menu> ExtendMenu(const std::string& name) = 0;

        virtual bool IsMenuRegistered(const std::string& name) const = 0;

        virtual std::shared_ptr<Menu> NewMenu(
            const std::string& baseName,
            const std::string& menuName,
            const MenuContext* context = {}) = 0;

        virtual std::shared_ptr<Menu> NewSubMenu(
            const std::shared_ptr<Menu>& parent,
            const std::string& baseName,
            const std::string& menuName) = 0;

        virtual std::shared_ptr<Menu> GenerateMenu(const std::string& name, const MenuContext& context) = 0;

        virtual std::shared_ptr<Menu> GenerateSubMenu(const std::shared_ptr<Menu>& parent, const std::string& subMenuName) = 0;

        virtual std::vector<std::shared_ptr<Menu>> GetGeneratedMenus() const = 0;
    };

    class MenuManager : public IMenuManager
    {
        PHX_DECLARE_TYPE_DERIVED(MenuManager, IMenuManager)

    public:

        void Initialize(const std::shared_ptr<Phoenix::Editor>& editor) override;
        void Shutdown() override;

        virtual std::shared_ptr<Menu> RegisterMenu(const std::string& name, const std::string& parentName = {}) override;

        virtual std::shared_ptr<Menu> FindMenu(const std::string& name) const override;

        virtual std::shared_ptr<Menu> ExtendMenu(const std::string& name) override;

        virtual bool IsMenuRegistered(const std::string& name) const override;

        virtual std::shared_ptr<Menu> NewMenu(
            const std::string& baseName,
            const std::string& menuName,
            const MenuContext* context = {}) override;

        virtual std::shared_ptr<Menu> NewSubMenu(
            const std::shared_ptr<Menu>& parent,
            const std::string& baseName,
            const std::string& menuName) override;

        virtual std::shared_ptr<Menu> GenerateMenu(const std::string& name, const MenuContext& context) override;

        virtual std::shared_ptr<Menu> GenerateSubMenu(const std::shared_ptr<Menu>& parent, const std::string& subMenuName) override;

        virtual std::vector<std::shared_ptr<Menu>> GetGeneratedMenus() const override;

    private:

        std::vector<std::shared_ptr<Menu>> GetMenuHierarchy(const std::string& name) const;

        std::shared_ptr<IMenuManager> ParentMenuManager;
        std::unordered_map<std::string, std::shared_ptr<Menu>> RegisteredMenus;
        std::unordered_map<std::string, std::shared_ptr<Menu>> GeneratedMenus;
        MenuAssembler Assembler;
    };
}
