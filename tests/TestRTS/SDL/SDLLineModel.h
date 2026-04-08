
#pragma once

#include <filesystem>

#include "PhoenixSim/Color.h"
#include "PhoenixSim/FixedPoint/FixedLine.h"
#include "PhoenixSim/FixedPoint/FixedTransform.h"

namespace Phoenix
{
    class ILogger;
}

struct SDLDebugRenderer;

struct LineModel
{
    std::vector<std::tuple<Phoenix::Color, std::vector<Phoenix::Line2>>> LineBatches;
    std::unordered_map<std::string, Phoenix::Transform2D> Sockets;
};

bool LoadLineModel(
    const std::filesystem::path& rootAssetPath,
    const std::filesystem::path& relativeFilePath,
    LineModel& outModel,
    Phoenix::ILogger* logger = nullptr);

void DrawLineModel(
    SDLDebugRenderer* renderer,
    const LineModel& model,
    const Phoenix::Transform2D& worldTransform,
    Phoenix::Angle assetScale = 1.0,
    Phoenix::Color tint = Phoenix::Color::White);