#include "SDL3AppServiceModule.h"

#include "SDL3App.h"
#include "SDL3PlatformService.h"
#include "SDL3Renderer.h"
#include "SDL3ImGuiService.h"
#include "Phoenix/Services/ServiceContainerBuilder.h"

void Phoenix::App::Dev::SDL3AppServiceModule::Register(ServiceContainerBuilder& builder) const
{
    // ReSharper disable CppExpressionWithoutSideEffects
    builder.Register<SDL3App>().AsInterfaces();
    builder.Register<SDL3PlatformService>().AsInterfaces();
    builder.Register<SDL3Renderer>().AsInterfaces();
    builder.Register<SDL3ResourceManager>().AsInterfaces();
    builder.Register<SDL3ImGuiService>().AsInterfaces();
    // ReSharper restore CppExpressionWithoutSideEffects
}
