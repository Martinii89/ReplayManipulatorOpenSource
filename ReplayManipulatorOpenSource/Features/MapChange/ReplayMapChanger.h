#pragma once
#include "Framework/GuiFeatureBase.h"

class ReplayMapChanger final : public GuiFeatureBase
{
public:
    struct Map
    {
        std::string name;
        std::string friendly_name;
        bool operator==(const Map&) const = default;
    };

    explicit ReplayMapChanger(std::shared_ptr<GameWrapper> gw);

    void ChangeMap(const Map& map) const;
    void UpdateCurrentMap();
    void Render() override;

private:
    bool DrawImGuiMapSelector(Map& selected_map) const;

    std::vector<Map> all_maps_;
    Map current_map_;
};
