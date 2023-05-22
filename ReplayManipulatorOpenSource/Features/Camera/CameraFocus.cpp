#include "pch.h"
#include "CameraFocus.h"

CameraFocus::CameraFocus(std::shared_ptr<GameWrapper> gw)
    : gw_(std::move(gw)) {}

void CameraFocus::FocusCameraOnPlayer(const std::string& actor_name) const
{
    if (!gw_->IsInReplay())
    {
        LOG("not in replay");
        return;
    }

    auto cam = gw_->GetCamera();
    if (!cam)
    {
        LOG("no camera?");
        return;
    }
    cam.SetCameraState("PlayerView");
    cam.SetFocusActor(std::format("Player_{}", actor_name));
}
