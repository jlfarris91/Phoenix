#include "PhoenixRTS/SmartValues/FeatureSmartValues.h"

Phoenix::RTS::FeatureSmartValues::FeatureSmartValues()
{
    FEATURE_WORLD_BLOCK(FeatureSmartValuesDynamicBlock)
    FEATURE_CHANNEL(FeatureChannels::HandleWorldAction)
}