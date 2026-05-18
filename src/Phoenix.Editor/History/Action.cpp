#include "Action.h"

#include <cassert>

uint32_t Phoenix::Action::GetId() const
{
    return ActionId;
}

std::string Phoenix::Action::GetDescription() const
{
    return Description.GetValue();
}

void Phoenix::Action::SetDescription(const Attribute<std::string>& description)
{
    Description = description;
}

Phoenix::EActionState Phoenix::Action::GetState() const
{
    return State;
}

bool Phoenix::Action::CanUndo() const
{
    return true;
}

void Phoenix::Action::Undo()
{
    assert(State == EActionState::Done);
    PerformUndo();
    State = EActionState::Undone;
}

bool Phoenix::Action::CanRedo() const
{
    return true;
}

void Phoenix::Action::Redo()
{
    assert(State == EActionState::Undone);
    PerformRedo();
    State = EActionState::Done;
}

void Phoenix::Action::AddDocumentToModify(const std::shared_ptr<IDocument>& document)
{
    if (std::ranges::find(DocumentsToModify, document) == DocumentsToModify.end())
    {
        DocumentsToModify.push_back(document);
    }
}

void Phoenix::Action::AddDocumentsToModify(const std::vector<std::shared_ptr<IDocument>>& documents)
{
    for (const auto& document : documents)
    {
        AddDocumentToModify(document);
    }
}

const std::vector<std::shared_ptr<Phoenix::IDocument>>& Phoenix::Action::GetDocumentsToModify() const
{
    return DocumentsToModify;
}
