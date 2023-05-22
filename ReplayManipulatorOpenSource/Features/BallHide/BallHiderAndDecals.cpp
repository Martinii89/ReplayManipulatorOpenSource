#include "pch.h"
#include "BallHiderAndDecals.h"


BallHiderAndDecals::BallHiderAndDecals(std::shared_ptr<GameWrapper> gw, std::shared_ptr<TextureCache> texture_cache, std::filesystem::path custom_decal_folder)
    : GuiFeatureBase(std::move(gw), "Ball Hider", DefaultVisibility::kWindow),
      custom_decal_folder_(std::move(custom_decal_folder)),
      texture_cache_(std::move(texture_cache))
{

    ReadConfigs(custom_decal_folder_);

    gw_->HookEventPost("Function TAGame.Ball_TA.EventGameEventSet",
                       [this](...) {
                           OnBallGameEventSet();
                       });
}

void BallHiderAndDecals::SetBallHidden(const bool hide)
{
    auto ball = GetBallWrapper();
    if (ball.IsNull())
    {
        DEBUGLOG("Failed to get the ball to hide");
        return;
    }
    ball_hidden_ = hide;
    ball.SetHidden2(hide);
}

void BallHiderAndDecals::FadeOutBall() const
{
    auto ball = GetBallWrapper();
    if (!ball)
    {
        DEBUGLOG("Failed to get the ball to hide");
        return;
    }
    ball.FadeOutBall();
}

void BallHiderAndDecals::FadeInBall()
{
    auto ball = GetBallWrapper();
    if (!ball)
    {
        DEBUGLOG("Failed to get the ball to hide");
        return;
    }
    ball.FadeInBall();
    if (!custom_ball_decal_.empty())
    {
        SetBallDecal(custom_ball_decal_);
    }
}

CustomBallDecal& BallHiderAndDecals::GetCustomDecal(const std::string& decal_name)
{
    if (const auto it = loaded_custom_decals_.find(decal_name); it == loaded_custom_decals_.end())
    {
        const auto it2 = custom_decal_configs_.find(decal_name);
        if (it2 == custom_decal_configs_.end())
        {
            return CustomTextures::default_ball_;
        }
        loaded_custom_decals_[decal_name] = CustomBallDecal(it2->second, texture_cache_);
    }

    return loaded_custom_decals_[decal_name];
}


void BallHiderAndDecals::Render()
{
    if (ImGui::Checkbox("Hidden", &ball_hidden_))
    {
        OnGameThread([this] {
            SetBallHidden(ball_hidden_);
        });
    }
    ImGui::TextUnformatted("Fading the ball hides the ball without breaking ballcam!");
    if (ImGui::Button("Fade out"))
    {
        OnGameThread([this] {
            FadeOutBall();
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Fade In"))
    {
        OnGameThread([this] {
            FadeInBall();
        });
    }

    std::string preview = custom_ball_decal_;
    if (preview.empty())
    {
        preview = "Select a ball decal";
    }
    char input_buffer[64] = "";
    if (ImGui::BeginSearchableCombo("##TheCustomDecalDropdown", preview.c_str(), input_buffer, 64,
                                    "search for the decal name"))
    {
        for (auto& [name, val] : custom_decal_configs_)
        {
            auto name2 = name + (val.has_invalid_paths ? " (has invalid paths)" : "");

            if (ImGui::Selectable(name2.c_str(), name == custom_ball_decal_))
            {
                SetBallDecal(name);
            }
        }
        ImGui::EndSearchableCombo();
    }
}


void BallHiderAndDecals::SetBallDecal(const std::string& name)
{
    custom_ball_decal_ = name;
    OnGameThread([this]() {
        if (auto the_ball = GetBallWrapper())
        {
            CustomTextures::ApplyTextureToBall(GetCustomDecal(custom_ball_decal_), the_ball);
        }
        else
        {
            LOG("no ball?");
        }
    });
}

void BallHiderAndDecals::ReadConfigs(const std::filesystem::path& json_folder)
{
    custom_decal_configs_.clear();
    if (!exists(json_folder))
    {
        LOG("the path for custom ball decals does not exist: {}", json_folder.string());
        return;
    }
    const auto ball_jsons = CustomTextures::FindJsons(json_folder);
    for (auto& p : ball_jsons)
    {
        auto [ball_configs] = CustomTextures::ReadCustomBallJsons(p, json_folder);
        for (auto& [name, ball_config] : ball_configs)
        {
            custom_decal_configs_[name] = ball_config;
        }
    }
}

BallWrapper BallHiderAndDecals::GetBallWrapper() const
{
    auto server = gw_->GetGameEventAsReplay();
    if (server.IsNull())
    {
        LOG("no server");
        return BallWrapper{0};
    }
    const auto ball = server.GetBall();
    return ball;
}

void BallHiderAndDecals::OnBallGameEventSet()
{
    gw_->SetTimeout([this](...) {
        if (!custom_ball_decal_.empty())
        {
            SetBallDecal(custom_ball_decal_);
        }
    }, 1);
}
