#pragma once

#include "Phoenix/Reflection/Registration.h"

#include "Util/Attribute.h"

namespace Phoenix
{
    class IDocument;

    enum class EActionState : uint8_t
    {
        Undone,
        Done
    };

    class Action : public std::enable_shared_from_this<Action>
    {
        PHX_DECLARE_TYPE_INTERFACE(Action)
    public:
        virtual ~Action() = default;

        uint32_t GetId() const;

        std::string GetDescription() const;
        void SetDescription(const Attribute<std::string>& description);

        EActionState GetState() const;

        virtual bool CanUndo() const;

        void Undo();

        virtual bool CanRedo() const;

        void Redo();

        virtual void AddDocumentToModify(const std::shared_ptr<IDocument>& document);

        virtual void AddDocumentsToModify(const std::vector<std::shared_ptr<IDocument>>& documents);

        virtual const std::vector<std::shared_ptr<IDocument>>& GetDocumentsToModify() const;

    protected:

        friend class IActionManager;

        virtual void PerformUndo() = 0;
        virtual void PerformRedo() = 0;

        uint32_t ActionId = 0;
        EActionState State = EActionState::Undone;
        Attribute<std::string> Description;
        std::vector<std::shared_ptr<IDocument>> DocumentsToModify;
    };
}
