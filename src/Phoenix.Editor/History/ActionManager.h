#pragma once

#include "Services/Service.h"
#include "Phoenix/Delegates.h"

namespace Phoenix
{
    class Action;

    PHX_DECLARE_MULTICAST_DELEGATE(ActionEvent, std::shared_ptr<const Action>);
    typedef std::function<bool(const std::shared_ptr<const Action>&)> CanPerformActionFunc;

    class IActionManager : public IService
    {
        PHX_DECLARE_TYPE_DERIVED(IActionManager, IService)
    public:

        virtual bool CanPerformAction(const std::shared_ptr<Action>& action) const;

        virtual bool AddAction(const std::shared_ptr<Action>& action) = 0;
        virtual bool AddActions(const std::vector<std::shared_ptr<Action>>& actions) = 0;

        virtual bool CanUndo() const = 0;
        virtual bool CanRedo() const = 0;

        virtual void Undo() = 0;
        virtual void Redo() = 0;

        virtual ActionEvent& OnActionPerformedEvent() = 0;
        virtual ActionEvent& OnUndoAction() = 0;
        virtual ActionEvent& OnRedoAction() = 0;

        virtual uint32_t GetUndoStack(std::vector<std::shared_ptr<const Action>>& outActions) const = 0;
        virtual uint32_t GetRedoStack(std::vector<std::shared_ptr<const Action>>& outActions) const = 0;

        virtual uint32_t GetCurrentActionId() const = 0;

    protected:

        static void SetActionId(Action& action, uint32_t id);
    };

    // Default implementation of an action manager
    class ActionManager : public IActionManager
    {
        PHX_DECLARE_TYPE_DERIVED(ActionManager, IActionManager)
    public:

        ActionManager(CanPerformActionFunc onCanPerformAction = {});

        bool CanPerformAction(const std::shared_ptr<Action>& action) const override;

        bool AddAction(const std::shared_ptr<Action>& action) override;
        bool AddActions(const std::vector<std::shared_ptr<Action>>& actions) override;

        bool CanUndo() const override;
        bool CanRedo() const override;

        void Undo() override;
        void Redo() override;

        ActionEvent& OnActionPerformedEvent() override;
        ActionEvent& OnUndoAction() override;
        ActionEvent& OnRedoAction() override;

        uint32_t GetUndoStack(std::vector<std::shared_ptr<const Action>>& outActions) const override;
        uint32_t GetRedoStack(std::vector<std::shared_ptr<const Action>>& outActions) const override;

        uint32_t GetCurrentActionId() const override;

    private:

        void ResetStack(std::vector<std::shared_ptr<Action>>& stack);

        std::vector<std::shared_ptr<Action>> UndoStack;
        std::vector<std::shared_ptr<Action>> RedoStack;

        CanPerformActionFunc OnCanPerformAction;
        ActionEvent ActionPerformedEvent;
        ActionEvent RedoActionEvent;
        ActionEvent UndoActionEvent;

        uint32_t ActionIdGen = 0;
    };
}
