#pragma once
#include "Features/CustomTextures/CustomTextures.h"
#include "Framework/GuiFeatureBase.h"

class BallHiderAndDecals final : public GuiFeatureBase
{
public:
    explicit BallHiderAndDecals(std::shared_ptr<GameWrapper> gw, std::shared_ptr<TextureCache> texture_cache, std::filesystem::path custom_decal_folder);
    void Render() override;

private:
    void SetBallHidden(bool hide);
    void FadeOutBall() const;
    void FadeInBall();
    CustomBallDecal& GetCustomDecal(const std::string& decal_name);
    void SetBallDecal(const std::string& name);
    void ReadConfigs(const std::filesystem::path& json_folder);

    [[nodiscard]] BallWrapper GetBallWrapper() const;

    void OnBallGameEventSet();

    bool ball_hidden_ = false;

    std::string custom_ball_decal_;
    std::filesystem::path custom_decal_folder_;
    std::map<std::string, CustomBallConfig> custom_decal_configs_;
    std::map<std::string, CustomBallDecal> loaded_custom_decals_;
    std::shared_ptr<TextureCache> texture_cache_;

};
