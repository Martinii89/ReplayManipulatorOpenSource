#pragma once
#include "Framework/GuiFeatureBase.h"


class CamPathsManager final : public GuiFeatureBase
{
public:
    CamPathsManager(std::shared_ptr<GameWrapper> gw, std::shared_ptr<CVarManagerWrapper> cw,
                    std::filesystem::path campaths_folder);

    void Render() override;

private:
    [[nodiscard]] std::vector<std::string> GetJsonsFilesInFolder(const std::filesystem::path& folder) const;
    std::filesystem::path campaths_folder_;
    std::vector<std::string> campaths_;
    std::shared_ptr<CVarManagerWrapper> cw_;
};
