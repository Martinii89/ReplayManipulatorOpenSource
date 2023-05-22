#pragma once

class CameraFocus
{
public:
    explicit CameraFocus(std::shared_ptr<GameWrapper> gw);

    void FocusCameraOnPlayer(const std::string& actor_name) const;

private:
    std::shared_ptr<GameWrapper> gw_;
};
