#include "pch.h"
#include "ReplayMapChanger.h"
#include "bakkesmod/wrappers/GameObject/ReplayManagerWrapper.h"
#include "bakkesmod/wrappers/GameObject/MapListWrapper.h"
#include "bakkesmod/wrappers/GameObject/MapDataWrapper.h"

ReplayMapChanger::ReplayMapChanger(std::shared_ptr<GameWrapper> gw)
    : GuiFeatureBase{std::move(gw), "Map Changer", DefaultVisibility::kBoth}
{
    if (const auto map_list_wrapper = gw_->GetMapListWrapper())
    {
        for (auto map : map_list_wrapper.GetSortedMaps())
        {
            const auto friendly_name = map.GetLocalizedName();
            const auto name = map.GetName();
            all_maps_.push_back({name, friendly_name});
        }
        std::ranges::sort(all_maps_, {}, &Map::friendly_name);
    }
}

bool ReplayMapChanger::DrawImGuiMapSelector(Map& selected_map) const
{
    bool map_chosen = false;
    char input_buffer[64] = "";
    if (ImGui::BeginSearchableCombo("Select map", current_map_.friendly_name.c_str(), input_buffer, 64,
                                    "Search for map"))
    {
        const auto filter_string = std::string(input_buffer);
        for (const auto& map : all_maps_)
        {
            auto find = std::ranges::search(map.friendly_name, filter_string,
                                            [](const char a, const char b) {
                                                return std::toupper(a) == std::toupper(b);
                                            }).begin();
            if (!filter_string.empty() && find == map.friendly_name.end())
                continue;
            const bool selected = map == current_map_;
            if (ImGui::Selectable(map.friendly_name.c_str(), selected))
            {
                map_chosen = true;
                selected_map = map;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndSearchableCombo();
    }
    return map_chosen;
}

void ReplayMapChanger::ChangeMap(const Map& map) const
{
    auto server = gw_->GetGameEventAsReplay();
    if (server.IsNull())
    {
        LOG("no server");
        return;
    }
    auto replay_wrapper = server.GetReplay();
    if (!replay_wrapper)
    {
        LOG("no replay");
        return;
    }

    //auto current_time = replay_wrapper.GetCurrentTime();
    auto current_frame = replay_wrapper.GetCurrentFrame();
    auto current_time = current_frame / replay_wrapper.GetRecordFPS();

    ReplayManagerWrapper const replay_manager = gw_->GetReplayManagerWrapper();
    DEBUGLOG("calling play replay with {} and time {}", map.name, current_time);
    replay_manager.PlayReplay(replay_wrapper, map.name, current_time);
}

void ReplayMapChanger::UpdateCurrentMap()
{
    auto current_map_name = gw_->GetCurrentMap();
    auto it = std::ranges::find_if(all_maps_, [current_map_name](const Map& map) {
        return map.name == current_map_name;
    });
    if (it != all_maps_.end())
    {
        current_map_ = *it;
    }
    else
    {
        current_map_ = {};
    }
}

void ReplayMapChanger::Render()
{
    static Map map;
    if (DrawImGuiMapSelector(map))
    {
        OnGameThread([this, map = map] {
            ChangeMap(map);
        });
    }
}
