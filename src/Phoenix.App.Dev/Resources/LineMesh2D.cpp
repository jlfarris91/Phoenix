#include "LineMesh2D.h"

#include <glm/gtc/matrix_transform.hpp>

#include "nlohmann/json.hpp"
#include "Phoenix/Color.h"
#include "Phoenix/Logging.h"

Phoenix::FName Phoenix::App::Dev::LineMesh2D::GetResourceType() const
{
    return StaticTypeName<LineMesh2D>().TypeId;
}

bool Phoenix::App::Dev::LineMesh2DLoader::CanLoad(const Renderer::ResourceLoadArgs& args) const
{
    nlohmann::json json;

    try
    {
        json = nlohmann::json::parse(args.Stream, nullptr, true);
    }
    catch (const nlohmann::detail::exception&)
    {
        return false;
    }

    return json.contains("format") && json["format"].get<std::string>() == "line_list_2d";
}

size_t Phoenix::App::Dev::LineMesh2DLoader::Load(
    const Renderer::ResourceLoadArgs& args,
    std::vector<std::unique_ptr<Renderer::IResource>>& outResources)
{
    nlohmann::json json;

    try
    {
        json = nlohmann::json::parse(args.Stream, nullptr, true);
    }
    catch (const nlohmann::detail::exception& ex)
    {
        LogError("Failed to load line model from stream: {}", ex.what());
        return false;
    }

    LineMesh2D lineMesh2D;

    std::vector<glm::vec4> colors;
    for (auto& colorJson : json["colors"])
    {
        std::string hexStr = colorJson.get<std::string>();
        Color color = Color::FromHex(hexStr.c_str());
        colors.emplace_back((float)color.R / 255.0f, (float)color.G / 255.0f, (float)color.B / 255.0f, (float)color.A / 255.0f);
    }

    nlohmann::json& data = json["data"];
    uint32 i = 0;
    while (i + 5 <= data.size())
    {
        uint32 colorIdx = data[i++].get<int>();
        auto index = lineMesh2D.Vertices.size();
        auto& v0 = lineMesh2D.Vertices.emplace_back();
        v0.Position.x = data[i++].get<float>();
        v0.Position.y = data[i++].get<float>();
        v0.Color = colors[colorIdx];
        auto& v1 = lineMesh2D.Vertices.emplace_back();
        v1.Position.x = data[i++].get<float>();
        v1.Position.y = data[i++].get<float>();
        v1.Color = colors[colorIdx];
        lineMesh2D.Indices.push_back(index + 0);
        lineMesh2D.Indices.push_back(index + 1);
    }

    if (json.contains("sockets"))
    {
        for (auto& socket : json["sockets"])
        {
            std::string id = socket["id"].get<std::string>();
            glm::vec3 position = { socket["x"].get<float>(), socket["y"].get<float>(), 0 };
            auto rotation = glm::degrees(socket["rotation"].get<float>());
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            lineMesh2D.Sockets.emplace(id, LineMesh2DSocket{ id, transform });
        }
    }

    outResources.push_back(std::make_unique<LineMesh2D>(std::move(lineMesh2D)));
    return 1;
}
