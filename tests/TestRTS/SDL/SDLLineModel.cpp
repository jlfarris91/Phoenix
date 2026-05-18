#include "SDLLineModel.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "SDLDebugRenderer.h"
#include "Phoenix.Sim/Logging.h"

using namespace Phoenix;
using PhoenixColor = Phoenix::Color;

bool LoadLineModel(
    const std::filesystem::path& rootAssetPath,
    const std::filesystem::path& relativeFilePath,
    LineModel& outModel,
    ILogger* logger)
{
    std::filesystem::path absoluteFilePath = absolute(rootAssetPath / relativeFilePath);

    std::ifstream fileStream(absoluteFilePath);
    if (!fileStream.is_open())
    {
        if (logger)
        {
            logger->LogError("Failed to open line model file '{}'", absoluteFilePath.string());
        }
        return false;
    }

    nlohmann::json json;

    try
    {
        json = nlohmann::json::parse(fileStream, nullptr, false);
    }
    catch (const nlohmann::detail::exception& ex)
    {
        if (logger)
        {
            logger->LogError("Failed to load line model file '{}': {}", absoluteFilePath.string(), ex.what());
        }
        return false;
    }

    LineModel model;

    for (auto& colorJson : json["colors"])
    {
        std::string hexStr = colorJson.get<std::string>();
        PhoenixColor color = PhoenixColor::FromHex(hexStr.c_str());
        model.LineBatches.emplace_back(color, std::vector<Line2>());
    }

    nlohmann::json& data = json["data"];
    uint32 i = 0;
    while (i + 5 <= data.size())
    {
        Line2 line;
        uint32 colorIdx = data[i++].get<int>();
        line.Start.X = data[i++].get<float>();
        line.Start.Y = data[i++].get<float>();
        line.End.X = data[i++].get<float>();
        line.End.Y = data[i++].get<float>();
        std::get<1>(model.LineBatches[colorIdx]).push_back(line);
    }

    if (json.contains("sockets"))
    {
        for (auto& socket : json["sockets"])
        {
            std::string id = socket["id"].get<std::string>();
            Transform2D transform;
            transform.Position.X = socket["x"].get<double>();
            transform.Position.Y = socket["y"].get<double>();
            transform.Rotation = socket["rotation"].get<double>();
            model.Sockets.emplace(id, transform);
        }
    }

    outModel = model;
    return true;
}

void DrawLineModel(
    SDLDebugRenderer* renderer,
    const LineModel& model,
    const Transform2D& worldTransform,
    Angle assetScale,
    PhoenixColor tint)
{
    LineModel worldModel = model;

    Transform2D modelTransform;
    modelTransform.Scale = assetScale;

    auto originSocketIter = worldModel.Sockets.find("origin");
    if (originSocketIter != worldModel.Sockets.end())
    {
        modelTransform.Position = -originSocketIter->second.Position * assetScale;
    }

    Transform2D modelToWorld = Transform2D::Combine(worldTransform, modelTransform);

    for (auto && [color, lines] : worldModel.LineBatches)
    {
        for (auto& line : lines)
        {
            line.Start = modelToWorld.TransformPoint(line.Start);
            line.End = modelToWorld.TransformPoint(line.End);
        }

        renderer->DrawLines(lines.data(), lines.size(), color * tint);
    }
}
