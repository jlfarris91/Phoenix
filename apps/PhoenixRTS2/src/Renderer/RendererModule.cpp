#include "RendererModule.h"

#include <filesystem>
#include <SDL3/SDL_asyncio.h>

#include "SDL3ResourceManager.h"
#include "Application/Application.h"
#include "Phoenix/Logging.h"

namespace fs = std::filesystem;

void RendererModule::Initialize(Phoenix::ModuleInitContext& context)
{
    IAppModule::Initialize(context);
}

void RendererModule::Load(Phoenix::ModuleLoadContext& context)
{
    IAppModule::Load(context);

    // Load assets here?

    auto resourceManager = GetApplication()->GetService<Phoenix::App::Dev::SDL3ResourceManager>();

    fs::path assetsDir = fs::absolute("./Data/Catalogs/Core/Assets");
    std::vector<Phoenix::Renderer::HResource> handles;

    try {
        if (fs::exists(assetsDir) && fs::is_directory(assetsDir))
        {
            // Iterates through everything inside the directory tree recursively
            for (const auto& entry : fs::recursive_directory_iterator(assetsDir))
            {
                // Filter out subdirectories if you only want regular files
                if (entry.is_regular_file())
                {
                    resourceManager->LoadResources(entry.path().generic_string().c_str(), handles);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        Phoenix::LogError("Error loading assets: {}", e.what());
    }
}
