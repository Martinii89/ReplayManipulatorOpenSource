#pragma once
#include "Data/PriUid.h"

struct CameraOverride
{
    bool enabled = false;
    ProfileCameraSettings override_settings{};
    ProfileCameraSettings original_settings{};
    PriUid id{""};

    [[nodiscard]] ProfileCameraSettings GetCameraSettings() const;
};


class CameraSettingsOverride
{
public:
    explicit CameraSettingsOverride(std::shared_ptr<GameWrapper> gw);

    [[nodiscard]] CameraOverride GetCameraOverrideSettings(const PriUid& id) const;
    void SetCameraOverrideSettings(const PriUid& id, const CameraOverride& camera_override_settings);

    //static ProfileCameraSettings GetPriPersistentCameraSettings(const PriWrapper& pri);
    static CameraOverride ReadOriginalSetting(PriWrapper& pri);
    static void SetPriCameraSetting(const PriWrapper& pri, const ProfileCameraSettings& settings);

    [[nodiscard]] static bool RenderCameraOverride(CameraOverride& camera_override);

private:
    void OnPersistentCameraSet(const CameraSettingsActorWrapper& camera_settings);

    std::unordered_map<PriUid, CameraOverride> camera_overrides_;
    std::shared_ptr<GameWrapper> gw_;
};
