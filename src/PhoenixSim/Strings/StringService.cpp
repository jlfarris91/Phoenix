#include "StringService.h"

using namespace Phoenix;

class DefaultStringService : public IStringService
{
public:
    const char* Get(const FName& name) const override
    {
        auto iter = Strings.find(name);
        return iter != Strings.end() ? iter->second.c_str() : nullptr;
    }

    const char* Store(const char* str, uint32 len) override
    {
        return StoreAs(str, len, FName(str, len));
    }

    const char* StoreAs(const char* str, uint32 len, const FName& name) override
    {
        if (const char* existing = Get(name))
        {
            return existing;
        }

        Strings.emplace(name, std::string(str, len));
        return Strings[name].c_str();
    }

private:
    std::unordered_map<FName, std::string> Strings;
} gDefaultStrings;

std::shared_ptr<IStringService> gPrimaryStringService;

IStringService& Phoenix::GetStringService()
{
    return gPrimaryStringService ? *gPrimaryStringService : gDefaultStrings;
}

void Phoenix::SetStringService(const std::shared_ptr<IStringService>& service)
{
    gPrimaryStringService = service;
}
