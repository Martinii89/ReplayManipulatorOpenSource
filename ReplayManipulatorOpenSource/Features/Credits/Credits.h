#pragma once
#include "Framework/GuiFeatureBase.h"

class CreditsInSettings final : public GuiFeatureBase
{
public:
    explicit CreditsInSettings(std::shared_ptr<GameWrapper> gw);

    void Render() override;
};
