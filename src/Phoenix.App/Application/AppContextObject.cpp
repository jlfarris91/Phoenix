#include "AppContextObject.h"

std::shared_ptr<Phoenix::Application> Phoenix::AppContextObject::GetApplication() const
{
    return WeakApp.lock();
}
