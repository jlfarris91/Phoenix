#include "Session.h"

Phoenix::Session::Session(const ConstructionParams& params)
    : Type(params.SessionType),
      Id(params.SessionId),
      Name(params.SessionName),
      RootDirectory(params.RootDirectory)
{
}

const std::string& Phoenix::Session::GetType() const
{
    return Type;
}

const std::string& Phoenix::Session::GetId() const
{
    return Id;
}

const std::string& Phoenix::Session::GetName() const
{
    return Name;
}

const std::string& Phoenix::Session::GetDisplayName() const
{
    // TODO (jfarris): Convert Name to a more user-friendly display name, e.g. by replacing underscores with spaces and capitalizing words.
    return Name;
}

const std::filesystem::path& Phoenix::Session::GetRootDirectory() const
{
    return RootDirectory;
}

bool Phoenix::Session::IsLoaded() const
{
    return bIsLoaded;
}

bool Phoenix::Session::IsLoading() const
{
    return bIsLoading;
}

bool Phoenix::Session::IsSaving() const
{
    return bIsSaving;
}

bool Phoenix::Session::IsNewSession() const
{
    return RootDirectory.empty();
}

std::filesystem::path Phoenix::Session::MakeAbsolutePath(const std::filesystem::path& relativePath) const
{
    return std::filesystem::absolute(RootDirectory / relativePath);
}

std::filesystem::path Phoenix::Session::MakeRelativePath(const std::filesystem::path& absolutePath) const
{
    return std::filesystem::relative(absolutePath, RootDirectory);
}

void Phoenix::Session::OnSessionLoaded(bool succeeded)
{
    // Currently this reports true if the load operation fails, if it was previously loaded successfully.
    // TODO (jfarris): Consider whether this is the desired behavior, or if we should set bIsLoaded to false if the load operation fails.
    bIsLoaded |= succeeded;

    bIsLoading = false;
    LastLoadResult = succeeded;
}

void Phoenix::Session::OnSessionSaved(bool succeeded)
{
    bIsSaving = false;
    LastSaveResult = succeeded;
}
