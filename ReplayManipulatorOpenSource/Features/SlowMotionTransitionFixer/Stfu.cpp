#include "pch.h"
#include "Stfu.h"
#include "bakkesmod/wrappers/Engine/WorldInfoWrapper.h"

SlowmotionTransitionFixerUtility::SlowmotionTransitionFixerUtility(std::shared_ptr<GameWrapper> gw)
    : GuiFeatureBase(std::move(gw), "Slowmotion Transition Fixer", DefaultVisibility::kSettings)
{
    gw_->HookEventWithCallerPost<ActorWrapper>(
        "Function ProjectX.CameraStateBlender_X.TransitionToState",
        [this](const ActorWrapper& caller, ...) {
            if (!enabled_)
                return;
            OnCameraBlenderTransition(CameraStateBlenderWrapper{caller.memory_address});
        });

    gw_->HookEventPost("Function TAGame.GFxData_ReplayViewer_TA.SetSlomo", [this](...) {
        if (!enabled_)
            return;
        OnSlomoChangedPost();
    });

    default_interp_values_ = DefaultInterpValues(CameraStateCarWrapper::GetInstanceWithDefaultValues());
}

void SlowmotionTransitionFixerUtility::Render()
{
    ImGui::Checkbox("Enabled", &enabled_);
}


void SlowmotionTransitionFixerUtility::OverrideInterpolationRates(const CameraStateBlenderWrapper& camera_blender,
                                                                  const float slomo) const
{
    auto car_state = CameraStateCarWrapper{camera_blender.GetCameraState().memory_address};
    if (!car_state || car_state.GetStateType() != "UCameraState_Car_TA")
    {
        return;
    }

    const auto new_values = default_interp_values_ * slomo;

    car_state.SetInterpToGroundRate(new_values.interp_to_ground_rate);
    car_state.SetInterpToAirRate(new_values.interp_to_air_rate);
    car_state.SetGroundRotationInterpRate(new_values.ground_rotation_interp_rate);
    car_state.SetGroundRotationInterpRateWall(new_values.ground_rotation_interp_rate_wall);
    car_state.SetFOVInterpSpeed(new_values.fov_interp_speed);
    car_state.SetSupersonicFOVInterpSpeed(new_values.supersonic_fov_interp_speed);
    car_state.SetGroundNormalInterpRate(new_values.ground_normal_interp_rate);
}

void SlowmotionTransitionFixerUtility::OnCameraBlenderTransition(const CameraStateBlenderWrapper& blender) const
{
    auto game_event = gw_->GetGameEventAsReplay();
    if (!game_event)
    {
        return;
    }
    WorldInfoWrapper world_info = game_event.GetWorldInfo();
    if (!world_info)
    {
        return;
    }

    const auto slomo = world_info.GetDemoPlayTimeDilation();
    auto transition = blender.GetTransition();
    const auto old_blend_time = transition.blend_params.blend_time;
    const auto slomo_adjusted = old_blend_time / slomo;
    transition.blend_params.blend_time = slomo_adjusted;
    transition.remaining_time = slomo_adjusted;
    blender.SetTransition(transition);
    OverrideInterpolationRates(blender, slomo);
}

void SlowmotionTransitionFixerUtility::OnSlomoChangedPost() const
{
    auto game_event = gw_->GetGameEventAsReplay();
    if (!game_event)
    {
        return;
    }
    WorldInfoWrapper world_info = game_event.GetWorldInfo();
    if (!world_info)
    {
        return;
    }
    auto slomo = world_info.GetDemoPlayTimeDilation();
    auto cam = gw_->GetCamera();
    if (!cam)
    {
        return;
    }
    const auto blender = cam.GetBlender();
    if (!blender)
    {
        return;
    }

    OverrideInterpolationRates(blender, slomo);
}


SlowmotionTransitionFixerUtility::DefaultInterpValues::DefaultInterpValues(const CameraStateCarWrapper& default_obj)
{
    if (!default_obj)
    {
        LOG("Failed to initialize default interp values!");
        return;
    }
    interp_to_ground_rate = default_obj.GetInterpToGroundRate();
    interp_to_air_rate = default_obj.GetInterpToAirRate();
    ground_rotation_interp_rate = default_obj.GetGroundRotationInterpRate();
    ground_rotation_interp_rate_wall = default_obj.GetGroundRotationInterpRateWall();
    fov_interp_speed = default_obj.GetFOVInterpSpeed();
    supersonic_fov_interp_speed = default_obj.GetSupersonicFOVInterpSpeed();
    ground_normal_interp_rate = default_obj.GetGroundNormalInterpRate();
}

SlowmotionTransitionFixerUtility::DefaultInterpValues SlowmotionTransitionFixerUtility::DefaultInterpValues::operator*(
    const float f) const
{
    DefaultInterpValues copy;
    copy.interp_to_ground_rate = interp_to_ground_rate * f;
    copy.interp_to_air_rate = interp_to_air_rate * f;
    copy.ground_rotation_interp_rate = ground_rotation_interp_rate * f;
    copy.ground_rotation_interp_rate_wall = ground_rotation_interp_rate_wall * f;
    copy.fov_interp_speed = fov_interp_speed * f;
    copy.supersonic_fov_interp_speed = supersonic_fov_interp_speed * f;
    copy.ground_normal_interp_rate = ground_normal_interp_rate * f;
    return copy;
}
