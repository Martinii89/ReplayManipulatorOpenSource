#include "pch.h"
#include "CameraSettingsOverride.h"

#include "ImguiUtils.h"
#include "bakkesmod/wrappers/GameObject/CameraSettingsActorWrapper.h"

ProfileCameraSettings CameraOverride::GetCameraSettings() const
{
    return enabled ? override_settings : original_settings;
}

int ImFormatString(char* buf, size_t buf_size, const char* fmt, ...);

bool SliderFloatWithSteps(const char* label, float* v, const float v_min, const float v_max, const float v_step,
                          const char* display_format)
{
    if (!display_format)
        display_format = "%.3f";

    char text_buf[64] = {};
    ImFormatString(text_buf, IM_ARRAYSIZE(text_buf), display_format, *v);

    // Map from [v_min,v_max] to [0,N]
    const int count_values = static_cast<int>((v_max - v_min) / v_step);
    int v_i = static_cast<int>((*v - v_min) / v_step);
    const bool value_changed = ImGui::SliderInt(label, &v_i, 0, count_values, text_buf);

    // Remap from [0,N] to [v_min,v_max]
    *v = v_min + static_cast<float>(v_i) * v_step;
    return value_changed;
}

CameraSettingsOverride::CameraSettingsOverride(std::shared_ptr<GameWrapper> gw)
    : gw_(std::move(gw))
{
    gw_->HookEventWithCallerPost<ActorWrapper>("Function TAGame.CameraSettingsActor_TA.ReplicatedEvent",
                                               [this](const ActorWrapper& actor, ...) {
                                                   OnPersistentCameraSet(
                                                       CameraSettingsActorWrapper{actor.memory_address});
                                               });

    gw_->HookEvent("Function TAGame.GFxHUD_Replay_TA.Destroyed", [this](...) {
        camera_overrides_.clear();
    });
}

CameraOverride CameraSettingsOverride::GetCameraOverrideSettings(const PriUid& id) const
{
    if (const auto it = camera_overrides_.find(id); it != camera_overrides_.end())
    {
        return it->second;
    }
    return {};
}

void CameraSettingsOverride::SetCameraOverrideSettings(const PriUid& id, const CameraOverride& camera_override_settings)
{
    if (const auto it = camera_overrides_.find(id); it != camera_overrides_.end())
    {
        it->second = camera_override_settings;
    }
}


CameraOverride CameraSettingsOverride::ReadOriginalSetting(PriWrapper& pri)
{
    return CameraOverride{.enabled = false, .override_settings = pri.GetCameraSettings(),
                          .original_settings = pri.GetCameraSettings(), .id = PriUid(pri)};
}

void CameraSettingsOverride::SetPriCameraSetting(const PriWrapper& pri, const ProfileCameraSettings& settings)
{
    if (auto camera = pri.GetPersistentCamera())
    {
        camera.SetProfileSettings(settings);
    }
}

bool CameraSettingsOverride::RenderCameraOverride(CameraOverride& camera_override)
{
    bool changed = false;
    auto& [FOV, Height, Pitch, Distance, Stiffness, SwivelSpeed, TransitionSpeed] = camera_override.override_settings;

    changed |= ImGui::Checkbox("Enabled", &camera_override.enabled);
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        camera_override.override_settings = camera_override.original_settings;
        changed = true;
    }
    {
        auto disabled = ImGui::Disable(!camera_override.enabled);
        changed |= SliderFloatWithSteps("FOV", &FOV, 60, 110, 1, "%.0f");
        changed |= SliderFloatWithSteps("Distance", &Distance, 100, 400, 10, "%.2f");
        changed |= SliderFloatWithSteps("Height", &Height, 40, 200, 10, "%.2f");
        changed |= SliderFloatWithSteps("Angle", &Pitch, -15, 0, 1, "%.2f");
        changed |= SliderFloatWithSteps("Stiffness", &Stiffness, 0, 1, 0.05f, "%.2f");
        changed |= SliderFloatWithSteps("SwivelSpeed", &SwivelSpeed, 1, 10, 0.1f, "%.2f");
        changed |= SliderFloatWithSteps("TransitionSpeed", &TransitionSpeed, 1, 2, 0.1f, "%.2f");
    }

    return changed;
}

void CameraSettingsOverride::OnPersistentCameraSet(const CameraSettingsActorWrapper& camera_settings)
{
    auto pri = camera_settings.GetPri();
    if (!pri)
    {
        return;
    }
    const auto id = PriUid(pri);
    if (const auto it = camera_overrides_.find(PriUid(pri)); it == camera_overrides_.end())
    {
        camera_overrides_[id] = ReadOriginalSetting(pri);
    }
    else
    {
        SetPriCameraSetting(pri, it->second.GetCameraSettings());
    }
}
