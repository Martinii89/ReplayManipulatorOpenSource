#pragma once
#include "Framework/GuiFeatureBase.h"
#include "bakkesmod/wrappers/GameObject/CameraStates/CameraStateCarWrapper.h"
#include "bakkesmod/wrappers/GameObject/CameraStates/CameraStateBlenderWrapper.h"

class SlowmotionTransitionFixerUtility final : public GuiFeatureBase
{
public:
    explicit SlowmotionTransitionFixerUtility(std::shared_ptr<GameWrapper> gw);

    void Render() override;

private:
    struct DefaultInterpValues
    {
        float interp_to_ground_rate = -1;
        float interp_to_air_rate = -1;
        float ground_rotation_interp_rate = -1;
        float ground_rotation_interp_rate_wall = -1;
        float fov_interp_speed = -1;
        float supersonic_fov_interp_speed = -1;
        float ground_normal_interp_rate = -1;

        DefaultInterpValues() = default;

        explicit DefaultInterpValues(const CameraStateCarWrapper& default_obj);
        DefaultInterpValues operator*(float f) const;
    };

    void OverrideInterpolationRates(const CameraStateBlenderWrapper& camera_blender, float slomo) const;
    void OnCameraBlenderTransition(const CameraStateBlenderWrapper& blender) const;
    void OnSlomoChangedPost() const;

    DefaultInterpValues default_interp_values_;

    bool enabled_ = true;
};
