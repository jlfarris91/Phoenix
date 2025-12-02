
#include "Json/JsonDataSource.h"

using namespace Phoenix::LDS::Json;

Phoenix::TSharedPtr<JsonDataSource> JsonDataSource::LoadFromDirectory(const PHXString& directoryPath)
{
    // TODO (jfarris): implement loading from directory.
    return MakeShared<JsonDataSource>();
}

Phoenix::TSharedPtr<JsonDataSource> JsonDataSource::GetParent() const
{
    return Parent;
}

void JsonDataSource::SetParent(const TSharedPtr<JsonDataSource>& parent)
{
    Parent = parent;
}

bool JsonDataSource::RegisterType(const nlohmann::json& typeJson)
{
    auto idIter = typeJson.find("id");
    if (idIter == typeJson.end())
    {
        // Error: no id property
        return false;
    }

    PHXString id = idIter->get<PHXString>();
    if (Types.contains(id))
    {
        // Error: type with id already exists
        return false;
    }

    Types.emplace(id, typeJson);
    return true;
}

const Phoenix::TMap<std::string, nlohmann::basic_json<>>& JsonDataSource::GetRegisteredTypes() const
{
    return Types;
}

const nlohmann::json* JsonDataSource::FindType(const PHXString& typeId) const
{
    auto iter = Types.find(typeId);
    if (iter != Types.end())
    {
        return &iter->second;
    }
    if (Parent)
    {
        return Parent->FindType(typeId);
    }
    return nullptr;
}

const nlohmann::json* JsonDataSource::FindObject(const PHXString& objectId) const
{
    auto iter = Objects.find(objectId);
    if (iter != Objects.end())
    {
        return &iter->second;
    }
    if (Parent)
    {
        return Parent->FindObject(objectId);
    }
    return nullptr;
}

bool JsonDataSource::RegisterObject(const nlohmann::json& objectJson)
{
    auto idIter = objectJson.find("id");
    if (idIter == objectJson.end())
    {
        // Error: no id property
        return false;
    }

    PHXString id = idIter->get<PHXString>();
    if (Objects.contains(id))
    {
        // Error: object with id already exists
        return false;
    }

    Objects.emplace(id, objectJson);
    return true;
}

const Phoenix::TMap<std::string, nlohmann::basic_json<>>& JsonDataSource::GetRegisteredObjects() const
{
    return Objects;
}
